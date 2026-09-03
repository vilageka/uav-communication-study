#include "ns3/core-module.h"
#include "ns3/buildings-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/yans-wifi-helper.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

/*
 * UAV urban OLSR mesh AoI experiment
 * ----------------------------------
 *
 * Diese Datei uebertraegt das urbane Gebaeudeszenario aus
 * uav-urban-wifi-aoi.cc auf die OLSR-Mesh-Architektur.
 *
 * Betrachtete Architektur:
 * - 802.11/Wi-Fi Ad-hoc als Funkbasis.
 * - OLSR als proaktives Mesh-Routing-Protokoll.
 * - Positionsupdates werden per UDP-Unicast von jedem UAV an jedes andere UAV
 *   gesendet.
 * - Wenn Sender und Empfaenger nicht direkt in Funkreichweite sind, kann OLSR
 *   ueber Zwischen-UAVs einen Multi-Hop-Pfad nutzen.
 *
 * Urbaner Unterschied zur freien Mesh-Version:
 * - UAVs stehen nicht mehr auf einem freien Gitter, sondern auf Strassenachsen
 *   zwischen Gebaeudebloecken.
 * - Der Wi-Fi-Kanal nutzt HybridBuildingsPropagationLossModel.
 * - Die Gebaeudegeometrie wird als CSV ausgegeben, damit das Szenario spaeter
 *   reproduzierbar bleibt.
 *
 * Gemessene Metriken:
 * - Packet Delivery Ratio.
 * - Ende-zu-Ende-Latenz.
 * - Age of Information.
 * - Kommunikationsaufwand auf Anwendungsebene in Paketen und Bytes.
 * - Hop-Anzahl als erste Naeherung ueber den empfangenen IPv4-TTL-Wert.
 *
 * Hinweis zur Hop-Anzahl:
 * Der Sender setzt eine feste initiale TTL. Am Empfaenger wird die verbleibende
 * TTL aus dem Socket-Tag gelesen. Daraus wird:
 *
 *   hopCount = initialTtl - remainingTtl + 1
 *
 * Ein direkt empfangenes Paket hat damit hopCount=1. Ein Paket ueber einen
 * Zwischen-UAV hat hopCount=2. Diese Messung ist fuer IPv4-Unicast geeignet
 * und reicht als erste Topologie-Metrik fuer den Mesh-Vergleich.
 */
NS_LOG_COMPONENT_DEFINE("UavUrbanMeshOlsrAoi");

namespace
{

constexpr uint16_t UavPositionPort = 9100;

struct UrbanScenarioConfig
{
    uint32_t blocksX{3};
    uint32_t blocksY{3};
    double buildingLengthX{80.0};
    double buildingLengthY{80.0};
    double streetWidth{40.0};
    double buildingHeight{35.0};
    std::string buildingMetricsFile{"uav-urban-mesh-olsr-buildings.csv"};
};

struct PairDistanceStats
{
    double minPairDistanceMeters{0.0};
    double averagePairDistanceMeters{0.0};
    double averageNearestNeighborDistanceMeters{0.0};
    double maxPairDistanceMeters{0.0};
};

struct PacketStats
{
    uint32_t packetsSent{0};
    uint32_t packetsReceived{0};
    uint64_t appBytesSent{0};
    uint64_t appBytesReceived{0};
    double latencySumMs{0.0};
    double latencyMinMs{0.0};
    double latencyMaxMs{0.0};
};

struct HopStats
{
    uint32_t samples{0};
    uint32_t minHopCount{0};
    uint32_t maxHopCount{0};
    double hopCountSum{0.0};
};

struct AoiStats
{
    uint64_t knownSamples{0};
    uint64_t unknownSamples{0};
    double aoiSumSeconds{0.0};
    double aoiMaxSeconds{0.0};
};

std::vector<std::vector<Ptr<Socket>>> g_sendSockets;
std::vector<std::vector<uint32_t>> g_sequenceNumbers;
std::vector<std::vector<double>> g_lastGenerationTime;
std::vector<std::vector<bool>> g_knownInformation;

PacketStats g_packetStats;
HopStats g_hopStats;
AoiStats g_aoiStats;

std::ofstream g_updateMetrics;
std::ofstream g_aoiMetrics;
std::ofstream g_buildingMetrics;

Time g_updateInterval;
Time g_aoiSampleInterval;
Time g_appStartTime;
Time g_stopTime;
uint32_t g_updatesPerPair{0};
uint8_t g_initialTtl{64};
bool g_verbose{false};

std::vector<Ptr<Building>>
CreateUrbanBuildingGrid(const UrbanScenarioConfig& urban)
{
    std::vector<Ptr<Building>> buildings;

    for (uint32_t xIndex = 0; xIndex < urban.blocksX; ++xIndex)
    {
        for (uint32_t yIndex = 0; yIndex < urban.blocksY; ++yIndex)
        {
            const double xMin =
                urban.streetWidth / 2.0 + xIndex * (urban.buildingLengthX + urban.streetWidth);
            const double xMax = xMin + urban.buildingLengthX;
            const double yMin =
                urban.streetWidth / 2.0 + yIndex * (urban.buildingLengthY + urban.streetWidth);
            const double yMax = yMin + urban.buildingLengthY;

            Ptr<Building> building = CreateObject<Building>();
            building->SetBoundaries(Box(xMin, xMax, yMin, yMax, 0.0, urban.buildingHeight));
            building->SetBuildingType(Building::Commercial);
            building->SetExtWallsType(Building::ConcreteWithWindows);
            building->SetNRoomsX(1);
            building->SetNRoomsY(1);
            building->SetNFloors(
                std::max<uint32_t>(1, static_cast<uint32_t>(urban.buildingHeight / 3.0)));
            buildings.push_back(building);
        }
    }

    return buildings;
}

void
WriteBuildingMetrics(const std::vector<Ptr<Building>>& buildings, const std::string& filename)
{
    g_buildingMetrics.open(filename);
    g_buildingMetrics << "building_id,x_min_m,x_max_m,y_min_m,y_max_m,z_min_m,z_max_m\n";

    for (Ptr<Building> building : buildings)
    {
        const Box box = building->GetBoundaries();
        g_buildingMetrics << building->GetId() << ',' << box.xMin << ',' << box.xMax << ','
                          << box.yMin << ',' << box.yMax << ',' << box.zMin << ',' << box.zMax
                          << '\n';
    }

    g_buildingMetrics.close();
}

std::string
BuildPositionMessage(uint32_t senderId,
                     uint32_t receiverId,
                     uint32_t sequence,
                     const Vector& position)
{
    std::ostringstream message;
    message << senderId << ' ' << receiverId << ' ' << sequence << ' '
            << Simulator::Now().GetSeconds() << ' ' << position.x << ' ' << position.y << ' '
            << position.z;
    return message.str();
}

bool
ParsePositionMessage(const std::string& message,
                     uint32_t& senderId,
                     uint32_t& receiverId,
                     uint32_t& sequence,
                     double& sendTimeSeconds,
                     Vector& position)
{
    std::istringstream input(message);
    input >> senderId >> receiverId >> sequence >> sendTimeSeconds >> position.x >> position.y >>
        position.z;
    return !input.fail();
}

void
RecordLatency(double latencyMs)
{
    g_packetStats.packetsReceived++;
    g_packetStats.latencySumMs += latencyMs;

    if (g_packetStats.packetsReceived == 1)
    {
        g_packetStats.latencyMinMs = latencyMs;
        g_packetStats.latencyMaxMs = latencyMs;
        return;
    }

    g_packetStats.latencyMinMs = std::min(g_packetStats.latencyMinMs, latencyMs);
    g_packetStats.latencyMaxMs = std::max(g_packetStats.latencyMaxMs, latencyMs);
}

void
RecordHopCount(uint32_t hopCount)
{
    g_hopStats.samples++;
    g_hopStats.hopCountSum += hopCount;

    if (g_hopStats.samples == 1)
    {
        g_hopStats.minHopCount = hopCount;
        g_hopStats.maxHopCount = hopCount;
        return;
    }

    g_hopStats.minHopCount = std::min(g_hopStats.minHopCount, hopCount);
    g_hopStats.maxHopCount = std::max(g_hopStats.maxHopCount, hopCount);
}

/*
 * AoI verwendet den neuesten bekannten Erzeugungszeitpunkt je receiver/sender.
 * Ein spaet eintreffendes aelteres Paket ist weiterhin ein erfolgreicher
 * Empfang, darf den Informationsstand aber nicht rueckwaerts aktualisieren.
 */
bool
ShouldAcceptInformationUpdate(uint32_t receiverId, uint32_t senderId, double sendTimeSeconds)
{
    return !g_knownInformation[receiverId][senderId] ||
           sendTimeSeconds > g_lastGenerationTime[receiverId][senderId];
}

uint32_t
ReadHopCount(Ptr<Packet> packet)
{
    SocketIpTtlTag ttlTag;
    if (!packet->RemovePacketTag(ttlTag))
    {
        return 0;
    }

    const uint8_t remainingTtl = ttlTag.GetTtl();
    if (remainingTtl > g_initialTtl)
    {
        return 0;
    }

    return static_cast<uint32_t>(g_initialTtl - remainingTtl + 1);
}

void
ReceivePositionUpdate(uint32_t localReceiverId, Ptr<Socket> socket)
{
    Address sourceAddress;
    Ptr<Packet> packet;

    while ((packet = socket->RecvFrom(sourceAddress)))
    {
        const uint32_t packetSize = packet->GetSize();
        const uint32_t hopCount = ReadHopCount(packet);

        std::string payload(packetSize, '\0');
        packet->CopyData(reinterpret_cast<uint8_t*>(&payload[0]), packetSize);

        uint32_t senderId;
        uint32_t receiverId;
        uint32_t sequence;
        double sendTimeSeconds;
        Vector position;
        if (!ParsePositionMessage(payload, senderId, receiverId, sequence, sendTimeSeconds, position))
        {
            NS_LOG_WARN("Ignoring malformed position update: " << payload);
            continue;
        }

        if (receiverId != localReceiverId)
        {
            NS_LOG_WARN("Ignoring packet for receiver " << receiverId << " at UAV "
                                                        << localReceiverId);
            continue;
        }

        const double nowSeconds = Simulator::Now().GetSeconds();
        const double latencyMs = (nowSeconds - sendTimeSeconds) * 1000.0;

        RecordLatency(latencyMs);
        if (hopCount > 0)
        {
            RecordHopCount(hopCount);
        }
        g_packetStats.appBytesReceived += packetSize;
        if (ShouldAcceptInformationUpdate(receiverId, senderId, sendTimeSeconds))
        {
            g_lastGenerationTime[receiverId][senderId] = sendTimeSeconds;
            g_knownInformation[receiverId][senderId] = true;
        }

        if (g_updateMetrics.is_open())
        {
            g_updateMetrics << std::fixed << std::setprecision(6) << nowSeconds << ','
                            << sendTimeSeconds << ',' << senderId << ',' << receiverId << ','
                            << sequence << ',' << latencyMs << ',' << hopCount << ','
                            << packetSize << ',' << position.x << ',' << position.y << ','
                            << position.z << '\n';
        }

        if (g_verbose)
        {
            std::cout << "t=" << nowSeconds << "s receiver=" << receiverId
                      << " sender=" << senderId << " seq=" << sequence
                      << " latency=" << latencyMs << "ms hops=" << hopCount << std::endl;
        }
    }
}

void
SampleAoi(NodeContainer uavs)
{
    const double nowSeconds = Simulator::Now().GetSeconds();

    for (uint32_t receiverId = 0; receiverId < uavs.GetN(); ++receiverId)
    {
        for (uint32_t senderId = 0; senderId < uavs.GetN(); ++senderId)
        {
            if (receiverId == senderId)
            {
                continue;
            }

            const bool known = g_knownInformation[receiverId][senderId];
            double aoiSeconds = -1.0;

            if (known)
            {
                aoiSeconds = nowSeconds - g_lastGenerationTime[receiverId][senderId];
                g_aoiStats.knownSamples++;
                g_aoiStats.aoiSumSeconds += aoiSeconds;
                g_aoiStats.aoiMaxSeconds = std::max(g_aoiStats.aoiMaxSeconds, aoiSeconds);
            }
            else
            {
                g_aoiStats.unknownSamples++;
            }

            if (g_aoiMetrics.is_open())
            {
                g_aoiMetrics << std::fixed << std::setprecision(6) << nowSeconds << ','
                             << receiverId << ',' << senderId << ',' << (known ? 1 : 0) << ','
                             << aoiSeconds << '\n';
            }
        }
    }

    if (Simulator::Now() + g_aoiSampleInterval <= g_stopTime)
    {
        Simulator::Schedule(g_aoiSampleInterval, &SampleAoi, uavs);
    }
}

void
SendPositionUpdate(NodeContainer uavs,
                   uint32_t senderId,
                   uint32_t receiverId,
                   uint32_t remainingUpdates)
{
    Ptr<MobilityModel> mobility = uavs.Get(senderId)->GetObject<MobilityModel>();
    Vector position = mobility->GetPosition();

    const uint32_t sequence = g_sequenceNumbers[senderId][receiverId]++;
    std::string message = BuildPositionMessage(senderId, receiverId, sequence, position);
    Ptr<Packet> packet =
        Create<Packet>(reinterpret_cast<const uint8_t*>(message.data()), message.size());

    g_sendSockets[senderId][receiverId]->Send(packet);
    g_packetStats.packetsSent++;
    g_packetStats.appBytesSent += message.size();

    if (remainingUpdates > 1)
    {
        Simulator::Schedule(g_updateInterval,
                            &SendPositionUpdate,
                            uavs,
                            senderId,
                            receiverId,
                            remainingUpdates - 1);
    }
}

Ptr<ListPositionAllocator>
CreateUrbanCorridorPositionAllocator(uint32_t numUavs,
                                     double spacingMeters,
                                     double altitudeMeters,
                                     const UrbanScenarioConfig& urban)
{
    Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator>();
    std::vector<double> verticalStreets;
    std::vector<double> horizontalStreets;
    std::vector<Vector> candidatePositions;

    for (uint32_t i = 0; i <= urban.blocksX; ++i)
    {
        if (i == 0)
        {
            verticalStreets.push_back(urban.streetWidth / 4.0);
        }
        else if (i == urban.blocksX)
        {
            verticalStreets.push_back(i * (urban.buildingLengthX + urban.streetWidth) -
                                      urban.streetWidth / 4.0);
        }
        else
        {
            verticalStreets.push_back(i * urban.buildingLengthX + (i + 0.5) * urban.streetWidth);
        }
    }
    for (uint32_t i = 0; i <= urban.blocksY; ++i)
    {
        if (i == 0)
        {
            horizontalStreets.push_back(urban.streetWidth / 4.0);
        }
        else if (i == urban.blocksY)
        {
            horizontalStreets.push_back(i * (urban.buildingLengthY + urban.streetWidth) -
                                        urban.streetWidth / 4.0);
        }
        else
        {
            horizontalStreets.push_back(i * urban.buildingLengthY + (i + 0.5) * urban.streetWidth);
        }
    }

    const double cityWidth =
        urban.blocksX * urban.buildingLengthX + (urban.blocksX + 1) * urban.streetWidth;
    const double cityHeight =
        urban.blocksY * urban.buildingLengthY + (urban.blocksY + 1) * urban.streetWidth;

    for (double y : horizontalStreets)
    {
        for (double x = urban.streetWidth / 4.0; x <= cityWidth; x += spacingMeters)
        {
            candidatePositions.push_back(Vector(x, y, altitudeMeters));
        }
    }

    for (double x : verticalStreets)
    {
        for (double y = urban.streetWidth / 4.0; y <= cityHeight; y += spacingMeters)
        {
            bool duplicate = false;
            for (const Vector& candidate : candidatePositions)
            {
                if (std::abs(candidate.x - x) < 1e-6 && std::abs(candidate.y - y) < 1e-6)
                {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate)
            {
                candidatePositions.push_back(Vector(x, y, altitudeMeters));
            }
        }
    }

    NS_ABORT_MSG_IF(candidatePositions.empty(), "urban position generator created no positions");

    for (uint32_t i = 0; i < numUavs; ++i)
    {
        const Vector base = candidatePositions[i % candidatePositions.size()];
        const uint32_t wrap = i / candidatePositions.size();
        positions->Add(Vector(base.x + wrap * 2.0, base.y, base.z));
    }

    return positions;
}

void
PrintUavPositions(const NodeContainer& uavs)
{
    std::cout << "Initial UAV positions" << std::endl;
    for (uint32_t i = 0; i < uavs.GetN(); ++i)
    {
        Ptr<MobilityModel> mobility = uavs.Get(i)->GetObject<MobilityModel>();
        Vector position = mobility->GetPosition();
        std::cout << "UAV " << i << ": (" << position.x << ", " << position.y << ", "
                  << position.z << ")" << std::endl;
    }
}

PairDistanceStats
ComputePairDistanceStats(const NodeContainer& uavs)
{
    PairDistanceStats stats;
    if (uavs.GetN() < 2)
    {
        return stats;
    }

    double pairDistanceSum = 0.0;
    uint64_t pairCount = 0;
    double nearestNeighborSum = 0.0;
    stats.minPairDistanceMeters = std::numeric_limits<double>::max();

    for (uint32_t i = 0; i < uavs.GetN(); ++i)
    {
        const Vector positionI = uavs.Get(i)->GetObject<MobilityModel>()->GetPosition();
        double nearestNeighbor = std::numeric_limits<double>::max();

        for (uint32_t j = 0; j < uavs.GetN(); ++j)
        {
            if (i == j)
            {
                continue;
            }

            const Vector positionJ = uavs.Get(j)->GetObject<MobilityModel>()->GetPosition();
            const double distanceMeters = CalculateDistance(positionI, positionJ);
            nearestNeighbor = std::min(nearestNeighbor, distanceMeters);

            if (j > i)
            {
                stats.minPairDistanceMeters =
                    std::min(stats.minPairDistanceMeters, distanceMeters);
                stats.maxPairDistanceMeters =
                    std::max(stats.maxPairDistanceMeters, distanceMeters);
                pairDistanceSum += distanceMeters;
                pairCount++;
            }
        }

        nearestNeighborSum += nearestNeighbor;
    }

    stats.averagePairDistanceMeters = pairDistanceSum / static_cast<double>(pairCount);
    stats.averageNearestNeighborDistanceMeters =
        nearestNeighborSum / static_cast<double>(uavs.GetN());
    return stats;
}

void
PrintUrbanScenario(const UrbanScenarioConfig& urban, uint32_t buildingCount)
{
    std::cout << "Urban scenario" << std::endl;
    std::cout << "Buildings: " << buildingCount << " (" << urban.blocksX << " x "
              << urban.blocksY << " blocks)" << std::endl;
    std::cout << "Building footprint: " << urban.buildingLengthX << " m x "
              << urban.buildingLengthY << " m" << std::endl;
    std::cout << "Street width: " << urban.streetWidth << " m" << std::endl;
    std::cout << "Building height: " << urban.buildingHeight << " m" << std::endl;
}

void
MakeUavBuildingInfoConsistent(const NodeContainer& uavs)
{
    for (uint32_t i = 0; i < uavs.GetN(); ++i)
    {
        Ptr<MobilityModel> mobility = uavs.Get(i)->GetObject<MobilityModel>();
        Ptr<MobilityBuildingInfo> buildingInfo = mobility->GetObject<MobilityBuildingInfo>();
        NS_ABORT_MSG_UNLESS(buildingInfo, "Missing MobilityBuildingInfo for UAV " << i);
        buildingInfo->MakeConsistent(mobility);
    }
}

} // namespace

int
main(int argc, char* argv[])
{
    uint32_t numUavs = 20;
    double trafficDurationSeconds = 10.0;
    double appStartSeconds = 5.0;
    double updateIntervalSeconds = 1.0;
    double aoiSampleIntervalSeconds = 0.1;
    double spacingMeters = 100.0;
    double altitudeMeters = 80.0;
    double txPowerDbm = 16.0;
    double channelFrequencyHz = 2.4e9;
    uint64_t rngRun = 1;
    bool enablePcap = false;
    std::string updateMetricsFile = "uav-mesh-olsr-aoi-updates.csv";
    std::string aoiMetricsFile = "uav-mesh-olsr-aoi-samples.csv";
    UrbanScenarioConfig urban;
    std::string phyMode = "DsssRate11Mbps";

    CommandLine cmd(__FILE__);
    cmd.AddValue("numUavs", "Number of UAV nodes", numUavs);
    cmd.AddValue("simTime", "Traffic duration in seconds", trafficDurationSeconds);
    cmd.AddValue("appStart", "Time before application traffic starts, allowing OLSR convergence", appStartSeconds);
    cmd.AddValue("updateInterval",
                 "Seconds between two position updates per sender/receiver pair",
                 updateIntervalSeconds);
    cmd.AddValue("aoiSampleInterval",
                 "Seconds between two AoI samples",
                 aoiSampleIntervalSeconds);
    cmd.AddValue("spacing", "Grid spacing between UAVs in meters", spacingMeters);
    cmd.AddValue("altitude", "UAV altitude in meters", altitudeMeters);
    cmd.AddValue("txPower", "Wi-Fi transmit power in dBm", txPowerDbm);
    cmd.AddValue("frequency", "Channel frequency in Hz for the urban propagation model", channelFrequencyHz);
    cmd.AddValue("rngRun", "ns-3 RNG run number for reproducible repetitions", rngRun);
    cmd.AddValue("blocksX", "Number of building blocks along the x axis", urban.blocksX);
    cmd.AddValue("blocksY", "Number of building blocks along the y axis", urban.blocksY);
    cmd.AddValue("buildingLengthX", "Building footprint length along x in meters", urban.buildingLengthX);
    cmd.AddValue("buildingLengthY", "Building footprint length along y in meters", urban.buildingLengthY);
    cmd.AddValue("streetWidth", "Width of streets between buildings in meters", urban.streetWidth);
    cmd.AddValue("buildingHeight", "Building height in meters", urban.buildingHeight);
    cmd.AddValue("initialTtl", "Initial IPv4 TTL used for hop-count estimation", g_initialTtl);
    cmd.AddValue("updateMetricsFile", "CSV file for received position updates", updateMetricsFile);
    cmd.AddValue("aoiMetricsFile", "CSV file for AoI samples", aoiMetricsFile);
    cmd.AddValue("buildingMetricsFile", "CSV file for generated building geometry", urban.buildingMetricsFile);
    cmd.AddValue("enablePcap", "Enable Wi-Fi pcap tracing", enablePcap);
    cmd.AddValue("verbose", "Print every received position update", g_verbose);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(numUavs < 2, "numUavs must be at least 2");
    NS_ABORT_MSG_IF(trafficDurationSeconds <= 0.0, "simTime must be positive");
    NS_ABORT_MSG_IF(appStartSeconds < 0.0, "appStart must not be negative");
    NS_ABORT_MSG_IF(updateIntervalSeconds <= 0.0, "updateInterval must be positive");
    NS_ABORT_MSG_IF(aoiSampleIntervalSeconds <= 0.0, "aoiSampleInterval must be positive");
    NS_ABORT_MSG_IF(g_initialTtl == 0, "initialTtl must be positive");
    NS_ABORT_MSG_IF(rngRun == 0, "rngRun must be positive");
    NS_ABORT_MSG_IF(urban.blocksX == 0 || urban.blocksY == 0, "blocksX and blocksY must be positive");
    NS_ABORT_MSG_IF(urban.buildingLengthX <= 0.0 || urban.buildingLengthY <= 0.0,
                    "building lengths must be positive");
    NS_ABORT_MSG_IF(urban.streetWidth <= 0.0, "streetWidth must be positive");
    NS_ABORT_MSG_IF(urban.buildingHeight <= 0.0, "buildingHeight must be positive");

    RngSeedManager::SetRun(rngRun);

    g_updateInterval = Seconds(updateIntervalSeconds);
    g_aoiSampleInterval = Seconds(aoiSampleIntervalSeconds);
    g_appStartTime = Seconds(appStartSeconds);
    g_stopTime = Seconds(appStartSeconds + trafficDurationSeconds + 1.0);
    g_updatesPerPair =
        static_cast<uint32_t>(std::floor(trafficDurationSeconds / updateIntervalSeconds));
    NS_ABORT_MSG_IF(g_updatesPerPair == 0, "simTime must allow at least one update per pair");

    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    NodeContainer uavs;
    uavs.Create(numUavs);

    std::vector<Ptr<Building>> buildings = CreateUrbanBuildingGrid(urban);
    WriteBuildingMetrics(buildings, urban.buildingMetricsFile);

    MobilityHelper mobility;
    mobility.SetPositionAllocator(
        CreateUrbanCorridorPositionAllocator(numUavs, spacingMeters, altitudeMeters, urban));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(uavs);
    BuildingsHelper::Install(uavs);
    MakeUavBuildingInfoConsistent(uavs);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue(phyMode),
                                 "ControlMode",
                                 StringValue(phyMode));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel",
                                   "Frequency",
                                   DoubleValue(channelFrequencyHz),
                                   "RooftopLevel",
                                   DoubleValue(urban.buildingHeight));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.Set("TxPowerStart", DoubleValue(txPowerDbm));
    wifiPhy.Set("TxPowerEnd", DoubleValue(txPowerDbm));
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, uavs);

    OlsrHelper olsr;
    Ipv4ListRoutingHelper routing;
    routing.Add(olsr, 10);

    InternetStackHelper internet;
    internet.SetRoutingHelper(routing);
    internet.Install(uavs);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.2.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    TypeId udpFactory = TypeId::LookupByName("ns3::UdpSocketFactory");
    g_sendSockets.assign(numUavs, std::vector<Ptr<Socket>>(numUavs));
    g_sequenceNumbers.assign(numUavs, std::vector<uint32_t>(numUavs, 0));
    g_lastGenerationTime.assign(numUavs, std::vector<double>(numUavs, 0.0));
    g_knownInformation.assign(numUavs, std::vector<bool>(numUavs, false));

    for (uint32_t receiverId = 0; receiverId < numUavs; ++receiverId)
    {
        Ptr<Socket> receiveSocket = Socket::CreateSocket(uavs.Get(receiverId), udpFactory);
        receiveSocket->SetIpRecvTtl(true);
        receiveSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UavPositionPort));
        receiveSocket->SetRecvCallback(MakeBoundCallback(&ReceivePositionUpdate, receiverId));
    }

    for (uint32_t senderId = 0; senderId < numUavs; ++senderId)
    {
        for (uint32_t receiverId = 0; receiverId < numUavs; ++receiverId)
        {
            if (senderId == receiverId)
            {
                continue;
            }

            Ptr<Socket> sendSocket = Socket::CreateSocket(uavs.Get(senderId), udpFactory);
            sendSocket->SetIpTtl(g_initialTtl);
            sendSocket->Connect(InetSocketAddress(interfaces.GetAddress(receiverId), UavPositionPort));
            g_sendSockets[senderId][receiverId] = sendSocket;
        }
    }

    g_updateMetrics.open(updateMetricsFile);
    g_updateMetrics << "receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,"
                    << "hop_count,payload_bytes,x_m,y_m,z_m\n";

    g_aoiMetrics.open(aoiMetricsFile);
    g_aoiMetrics << "time_s,receiver_id,sender_id,known,aoi_s\n";

    if (enablePcap)
    {
        wifiPhy.EnablePcap("uav-urban-mesh-olsr-aoi", devices);
    }

    PrintUrbanScenario(urban, buildings.size());
    PrintUavPositions(uavs);
    const PairDistanceStats distanceStats = ComputePairDistanceStats(uavs);

    Simulator::Schedule(Seconds(0.0), &SampleAoi, uavs);

    for (uint32_t senderId = 0; senderId < numUavs; ++senderId)
    {
        for (uint32_t receiverId = 0; receiverId < numUavs; ++receiverId)
        {
            if (senderId == receiverId)
            {
                continue;
            }

            const double offsetSeconds = (senderId * numUavs + receiverId) * 0.001;
            Simulator::Schedule(g_appStartTime + Seconds(offsetSeconds),
                                &SendPositionUpdate,
                                uavs,
                                senderId,
                                receiverId,
                                g_updatesPerPair);
        }
    }

    Simulator::Stop(g_stopTime);
    Simulator::Run();
    Simulator::Destroy();

    g_updateMetrics.close();
    g_aoiMetrics.close();

    const uint32_t expectedReceives = g_packetStats.packetsSent;
    const double deliveryRatio = expectedReceives > 0
                                     ? static_cast<double>(g_packetStats.packetsReceived) /
                                           static_cast<double>(expectedReceives)
                                     : 0.0;
    const double averageLatencyMs = g_packetStats.packetsReceived > 0
                                        ? g_packetStats.latencySumMs /
                                              static_cast<double>(g_packetStats.packetsReceived)
                                        : 0.0;
    const double averageHopCount = g_hopStats.samples > 0
                                       ? g_hopStats.hopCountSum /
                                             static_cast<double>(g_hopStats.samples)
                                       : 0.0;
    const uint64_t totalAoiSamples = g_aoiStats.knownSamples + g_aoiStats.unknownSamples;
    const double averageKnownAoiSeconds = g_aoiStats.knownSamples > 0
                                              ? g_aoiStats.aoiSumSeconds /
                                                    static_cast<double>(g_aoiStats.knownSamples)
                                              : 0.0;
    const double unknownAoiShare = totalAoiSamples > 0
                                       ? static_cast<double>(g_aoiStats.unknownSamples) /
                                             static_cast<double>(totalAoiSamples)
                                       : 0.0;

    std::cout << "\nSimulation summary" << std::endl;
    std::cout << "Architecture: Urban Wi-Fi ad hoc OLSR mesh with AoI sampling" << std::endl;
    std::cout << "UAVs: " << numUavs << std::endl;
    std::cout << "Buildings: " << buildings.size() << std::endl;
    std::cout << "Street width: " << urban.streetWidth << " m" << std::endl;
    std::cout << "Building height: " << urban.buildingHeight << " m" << std::endl;
    std::cout << "Min pair distance: " << distanceStats.minPairDistanceMeters << " m" << std::endl;
    std::cout << "Average pair distance: " << distanceStats.averagePairDistanceMeters << " m"
              << std::endl;
    std::cout << "Average nearest-neighbor distance: "
              << distanceStats.averageNearestNeighborDistanceMeters << " m" << std::endl;
    std::cout << "Max pair distance: " << distanceStats.maxPairDistanceMeters << " m" << std::endl;
    std::cout << "Traffic start: " << appStartSeconds << " s" << std::endl;
    std::cout << "Sent application packets: " << g_packetStats.packetsSent << std::endl;
    std::cout << "Received application packets: " << g_packetStats.packetsReceived << " / "
              << expectedReceives << std::endl;
    std::cout << "Delivery ratio: " << deliveryRatio << std::endl;
    std::cout << "Application bytes sent: " << g_packetStats.appBytesSent << std::endl;
    std::cout << "Application bytes received: " << g_packetStats.appBytesReceived << std::endl;
    std::cout << "Average latency: " << averageLatencyMs << " ms" << std::endl;
    std::cout << "Min latency: " << g_packetStats.latencyMinMs << " ms" << std::endl;
    std::cout << "Max latency: " << g_packetStats.latencyMaxMs << " ms" << std::endl;
    std::cout << "Average hop count: " << averageHopCount << std::endl;
    std::cout << "Min hop count: " << g_hopStats.minHopCount << std::endl;
    std::cout << "Max hop count: " << g_hopStats.maxHopCount << std::endl;
    std::cout << "Known AoI samples: " << g_aoiStats.knownSamples << std::endl;
    std::cout << "Unknown AoI samples: " << g_aoiStats.unknownSamples << std::endl;
    std::cout << "Unknown AoI share: " << unknownAoiShare << std::endl;
    std::cout << "Average known AoI: " << averageKnownAoiSeconds << " s" << std::endl;
    std::cout << "Max known AoI: " << g_aoiStats.aoiMaxSeconds << " s" << std::endl;
    std::cout << "Update metrics CSV: " << updateMetricsFile << std::endl;
    std::cout << "AoI metrics CSV: " << aoiMetricsFile << std::endl;
    std::cout << "Building metrics CSV: " << urban.buildingMetricsFile << std::endl;

    return 0;
}
