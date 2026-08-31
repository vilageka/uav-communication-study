#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

/*
 * UAV LTE infrastructure AoI experiment
 * -------------------------------------
 *
 * Diese Datei modelliert die LTE-basierte Infrastrukturarchitektur der
 * Bachelorarbeit.
 *
 * Betrachtete Architektur:
 * - Jedes UAV wird als LTE User Equipment (UE) modelliert.
 * - Eine eNodeB versorgt den UAV-Schwarm als zentrale Infrastruktur.
 * - Das EPC stellt den IP-Kern bereit.
 * - Positionsupdates werden per UDP-Unicast von jedem UAV an jedes andere UAV
 *   gesendet und laufen logisch ueber die LTE/EPC-Infrastruktur.
 *
 * Vergleich zu den bisherigen Architekturen:
 * - Broadcast sendet pro UAV und Intervall nur ein Paket, erreicht aber nur
 *   direkte Funknachbarn.
 * - OLSR-Mesh nutzt UAVs als Weiterleitungsknoten und erzeugt Multi-Hop-Pfade.
 * - LTE nutzt Infrastruktur statt UAV-zu-UAV-Weiterleitung. Dadurch haengt die
 *   Erreichbarkeit weniger von direkter Nachbarschaft zwischen UAVs ab, sondern
 *   vor allem von Zellabdeckung, Scheduling und EPC-Pfad.
 *
 * Gemessene Metriken:
 * - Packet Delivery Ratio.
 * - Ende-zu-Ende-Latenz.
 * - Age of Information.
 * - Kommunikationsaufwand auf Anwendungsebene in Paketen und Bytes.
 * - Infrastruktur-Hop-Schaetzung ueber den empfangenen IPv4-TTL-Wert.
 *
 * Hinweis zur Hop-Anzahl:
 * Diese Hop-Anzahl ist nicht direkt mit Mesh-Hop-Anzahl gleichzusetzen. Im
 * Mesh bedeutet ein Hop typischerweise eine Funkweiterleitung zwischen UAVs.
 * Bei LTE beschreibt sie den IP-Pfad durch die Infrastruktur. Sie bleibt
 * trotzdem nuetzlich, um zu dokumentieren, ob Pakete direkt oder ueber einen
 * laengeren Infrastrukturpfad empfangen wurden.
 *
 * Fuer einen wissenschaftlichen Vergleich sollte diese Spalte spaeter klar als
 * IP/infrastructure hop estimate bezeichnet werden, nicht als UAV-Mesh-Hop.
 */
NS_LOG_COMPONENT_DEFINE("UavLteInfrastructureAoi");

namespace
{

constexpr uint16_t UavPositionPort = 9200;

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

Time g_updateInterval;
Time g_aoiSampleInterval;
Time g_appStartTime;
Time g_stopTime;
uint32_t g_updatesPerPair{0};
uint8_t g_initialTtl{64};
bool g_verbose{false};

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
        g_lastGenerationTime[receiverId][senderId] = sendTimeSeconds;
        g_knownInformation[receiverId][senderId] = true;

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
CreateGridPositionAllocator(uint32_t numUavs, double spacingMeters, double altitudeMeters)
{
    Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator>();
    const uint32_t columns = static_cast<uint32_t>(std::ceil(std::sqrt(numUavs)));

    for (uint32_t i = 0; i < numUavs; ++i)
    {
        const uint32_t row = i / columns;
        const uint32_t column = i % columns;
        positions->Add(Vector(column * spacingMeters, row * spacingMeters, altitudeMeters));
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

} // namespace

int
main(int argc, char* argv[])
{
    uint32_t numUavs = 20;
    double trafficDurationSeconds = 10.0;
    double appStartSeconds = 1.0;
    double updateIntervalSeconds = 1.0;
    double aoiSampleIntervalSeconds = 0.1;
    double spacingMeters = 100.0;
    double altitudeMeters = 80.0;
    double enbHeightMeters = 30.0;
    bool enablePcap = false;
    bool enableLteTraces = false;
    std::string updateMetricsFile = "uav-lte-infrastructure-aoi-updates.csv";
    std::string aoiMetricsFile = "uav-lte-infrastructure-aoi-samples.csv";

    CommandLine cmd(__FILE__);
    cmd.AddValue("numUavs", "Number of UAV nodes", numUavs);
    cmd.AddValue("simTime", "Traffic duration in seconds", trafficDurationSeconds);
    cmd.AddValue("appStart", "Time before application traffic starts, allowing LTE attach", appStartSeconds);
    cmd.AddValue("updateInterval",
                 "Seconds between two position updates per sender/receiver pair",
                 updateIntervalSeconds);
    cmd.AddValue("aoiSampleInterval",
                 "Seconds between two AoI samples",
                 aoiSampleIntervalSeconds);
    cmd.AddValue("spacing", "Grid spacing between UAVs in meters", spacingMeters);
    cmd.AddValue("altitude", "UAV altitude in meters", altitudeMeters);
    cmd.AddValue("enbHeight", "Height of the central eNodeB in meters", enbHeightMeters);
    cmd.AddValue("initialTtl", "Initial IPv4 TTL used for infrastructure hop estimation", g_initialTtl);
    cmd.AddValue("updateMetricsFile", "CSV file for received position updates", updateMetricsFile);
    cmd.AddValue("aoiMetricsFile", "CSV file for AoI samples", aoiMetricsFile);
    cmd.AddValue("enablePcap", "Enable EPC point-to-point pcap tracing", enablePcap);
    cmd.AddValue("enableLteTraces", "Enable LTE PHY/MAC/RLC/PDCP trace files", enableLteTraces);
    cmd.AddValue("verbose", "Print every received position update", g_verbose);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(numUavs < 2, "numUavs must be at least 2");
    NS_ABORT_MSG_IF(trafficDurationSeconds <= 0.0, "simTime must be positive");
    NS_ABORT_MSG_IF(appStartSeconds < 0.0, "appStart must not be negative");
    NS_ABORT_MSG_IF(updateIntervalSeconds <= 0.0, "updateInterval must be positive");
    NS_ABORT_MSG_IF(aoiSampleIntervalSeconds <= 0.0, "aoiSampleInterval must be positive");
    NS_ABORT_MSG_IF(g_initialTtl == 0, "initialTtl must be positive");

    g_updateInterval = Seconds(updateIntervalSeconds);
    g_aoiSampleInterval = Seconds(aoiSampleIntervalSeconds);
    g_appStartTime = Seconds(appStartSeconds);
    g_stopTime = Seconds(appStartSeconds + trafficDurationSeconds + 1.0);
    g_updatesPerPair =
        static_cast<uint32_t>(std::floor(trafficDurationSeconds / updateIntervalSeconds));
    NS_ABORT_MSG_IF(g_updatesPerPair == 0, "simTime must allow at least one update per pair");

    NodeContainer uavs;
    uavs.Create(numUavs);

    MobilityHelper mobility;
    mobility.SetPositionAllocator(CreateGridPositionAllocator(numUavs, spacingMeters, altitudeMeters));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(uavs);

    /*
     * LTE-Infrastruktur:
     *
     * Eine einzelne eNodeB wird in die Mitte des UAV-Gitters gesetzt. Das ist
     * eine vereinfachte Infrastrukturannahme: alle UAVs teilen sich dieselbe
     * Funkzelle. Fuer Skalierungsexperimente ist das interessant, weil mit
     * wachsender UAV-Zahl mehr UEs um dieselben LTE-Ressourcen konkurrieren.
     */
    NodeContainer enbNodes;
    enbNodes.Create(1);

    const uint32_t gridColumns = static_cast<uint32_t>(std::ceil(std::sqrt(numUavs)));
    const uint32_t gridRows = static_cast<uint32_t>(std::ceil(static_cast<double>(numUavs) /
                                                              static_cast<double>(gridColumns)));
    const double enbX = (gridColumns - 1) * spacingMeters / 2.0;
    const double enbY = (gridRows - 1) * spacingMeters / 2.0;

    Ptr<ListPositionAllocator> enbPosition = CreateObject<ListPositionAllocator>();
    enbPosition->Add(Vector(enbX, enbY, enbHeightMeters));
    MobilityHelper enbMobility;
    enbMobility.SetPositionAllocator(enbPosition);
    enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbMobility.Install(enbNodes);

    // Optional PCAP tracing for the S1-U backhaul links created inside the
    // EPC helper. LTE radio traces are controlled separately by enableLteTraces.
    Config::SetDefault("ns3::PointToPointEpcHelper::S1uLinkEnablePcap",
                       BooleanValue(enablePcap));

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    InternetStackHelper internet;
    internet.Install(uavs);

    NetDeviceContainer enbLteDevices = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueLteDevices = lteHelper->InstallUeDevice(uavs);

    Ipv4InterfaceContainer interfaces = epcHelper->AssignUeIpv4Address(ueLteDevices);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    for (uint32_t i = 0; i < uavs.GetN(); ++i)
    {
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(uavs.Get(i)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    /*
     * Alle UEs werden an die zentrale eNodeB gebunden. Der Default Bearer wird
     * dabei automatisch aktiviert. Dedicated Bearer sind fuer diese Baseline
     * noch nicht noetig, koennen spaeter aber fuer QoS-Varianten relevant sein.
     */
    for (uint32_t i = 0; i < uavs.GetN(); ++i)
    {
        lteHelper->Attach(ueLteDevices.Get(i), enbLteDevices.Get(0));
    }

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

    if (enableLteTraces)
    {
        lteHelper->EnableTraces();
    }

    PrintUavPositions(uavs);

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
    std::cout << "Architecture: LTE infrastructure with AoI sampling" << std::endl;
    std::cout << "UAVs: " << numUavs << std::endl;
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
    std::cout << "Average infrastructure hop estimate: " << averageHopCount << std::endl;
    std::cout << "Min infrastructure hop estimate: " << g_hopStats.minHopCount << std::endl;
    std::cout << "Max infrastructure hop estimate: " << g_hopStats.maxHopCount << std::endl;
    std::cout << "Known AoI samples: " << g_aoiStats.knownSamples << std::endl;
    std::cout << "Unknown AoI samples: " << g_aoiStats.unknownSamples << std::endl;
    std::cout << "Unknown AoI share: " << unknownAoiShare << std::endl;
    std::cout << "Average known AoI: " << averageKnownAoiSeconds << " s" << std::endl;
    std::cout << "Max known AoI: " << g_aoiStats.aoiMaxSeconds << " s" << std::endl;
    std::cout << "Update metrics CSV: " << updateMetricsFile << std::endl;
    std::cout << "AoI metrics CSV: " << aoiMetricsFile << std::endl;

    return 0;
}
