#include "ns3/core-module.h"
#include "ns3/buildings-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"

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
 * UAV urban Wi-Fi AoI experiment
 * ------------------------------
 *
 * Diese Datei ist die erste konkrete urbane Szenarioversion des Projekts.
 * Sie verwendet bewusst weiterhin die einfache Wi-Fi-Ad-hoc-Broadcast-
 * Architektur aus uav-wifi-aoi.cc. Dadurch aendert sich in dieser Version vor
 * allem die Umgebung, nicht die Kommunikationslogik.
 *
 * Grundidee des urbanen Szenarios:
 *
 * - Gebaeude werden als rechteckige ns-3 Building-Objekte modelliert.
 * - Zwischen den Gebaeuden bleiben Strassenkorridore frei.
 * - UAVs werden entlang dieser Korridore positioniert.
 * - Der Funkkanal verwendet das HybridBuildingsPropagationLossModel. Dieses
 *   Modell bildet nicht jedes einzelne Hindernis als Raytracing-Geometrie ab,
 *   bringt aber urbanen Pfadverlust, Rooftop-Effekte und Gebaeudeinformationen
 *   in die Simulation.
 *
 * Warum zuerst Wi-Fi Ad-hoc?
 *
 * Wi-Fi Broadcast ist unsere ADS-L-inspirierte Referenz. Wenn dieses einfache
 * Verfahren im freien Feld sehr gut aussieht, aber in einer urbanen Umgebung
 * schlechter wird, ist das ein gutes Argument fuer Mesh- oder
 * Infrastrukturansaetze. Diese Datei schafft also die Szenario-Basis, die wir
 * danach auf OLSR und LTE uebertragen koennen.
 *
 * Gemessene Metriken bleiben:
 *
 * - Packet Delivery Ratio.
 * - Ende-zu-Ende-Latenz fuer erfolgreich empfangene Updates.
 * - Age of Information:
 *
 *     AoI(receiver, sender, t) = t - generationTime(lastUpdate(sender))
 *
 * - Anteil unbekannter AoI-Zustaende, also Faelle, in denen ein UAV ueber ein
 *   anderes UAV noch keine Information besitzt.
 *
 * Wichtige Einschraenkung:
 *
 * Dieses Modell ist noch kein vollstaendiger digitaler Stadtzwilling. Es ist
 * ein reproduzierbares, parametrisierbares Urban-Benchmark-Szenario. Genau das
 * brauchen wir jetzt, um Architekturunterschiede systematisch testen zu
 * koennen.
 */
NS_LOG_COMPONENT_DEFINE("UavUrbanWifiAoi");

namespace
{

// Alle UAVs nutzen denselben UDP-Port fuer Positionsupdates.
constexpr uint16_t UavPositionPort = 9000;

struct UrbanScenarioConfig
{
    uint32_t blocksX{3};
    uint32_t blocksY{3};
    double buildingLengthX{80.0};
    double buildingLengthY{80.0};
    double streetWidth{40.0};
    double buildingHeight{35.0};
    std::string buildingMetricsFile{"uav-urban-wifi-buildings.csv"};
};

/*
 * Sammelstruktur fuer paketbezogene Metriken.
 *
 * Diese Werte entsprechen im Wesentlichen der bisherigen Baseline:
 * Anzahl gesendeter Pakete, Anzahl empfangener Pakete und Latenzstatistik.
 */
struct PacketStats
{
    uint32_t packetsSent{0};
    uint32_t packetsReceived{0};
    double latencySumMs{0.0};
    double latencyMinMs{0.0};
    double latencyMaxMs{0.0};
};

/*
 * Sammelstruktur fuer AoI-Metriken.
 *
 * knownSamples:
 *   Anzahl der AoI-Samples, bei denen der Empfaenger bereits mindestens ein
 *   Update vom betrachteten Sender erhalten hatte.
 *
 * unknownSamples:
 *   Anzahl der Samples, bei denen noch gar keine Information vorhanden war.
 *   Diese Zustaende sind wichtig: Sie zeigen, dass ein UAV ueber einen anderen
 *   UAV noch keine Positionsinformation besitzt.
 */
struct AoiStats
{
    uint64_t knownSamples{0};
    uint64_t unknownSamples{0};
    double aoiSumSeconds{0.0};
    double aoiMaxSeconds{0.0};
};

std::vector<Ptr<Socket>> g_sendSockets;
std::vector<uint32_t> g_sequenceNumbers;

// lastGenerationTime[receiver][sender] speichert die Erzeugungszeit des
// letzten Updates, das receiver ueber sender erhalten hat.
std::vector<std::vector<double>> g_lastGenerationTime;

// knownInformation[receiver][sender] sagt, ob receiver ueberhaupt schon ein
// Update von sender erhalten hat. Ohne diese Matrix waere ein Zeitwert 0.0
// mehrdeutig: unbekannt oder ein echtes Update bei t=0.
std::vector<std::vector<bool>> g_knownInformation;

PacketStats g_packetStats;
AoiStats g_aoiStats;

std::ofstream g_updateMetrics;
std::ofstream g_aoiMetrics;
std::ofstream g_buildingMetrics;

Time g_updateInterval;
Time g_aoiSampleInterval;
Time g_stopTime;
uint32_t g_updatesPerUav{0};
bool g_verbose{false};

/*
 * Erzeugt eine einfache Block-Stadt mit orthogonalem Strassennetz.
 *
 * Koordinatenlogik:
 * - Jede Zelle besteht aus einem Gebaeudeblock plus Strassenabstand.
 * - Der Gebaeudeblock beginnt bei x/y = streetWidth / 2. Dadurch gibt es am
 *   linken und unteren Rand ebenfalls freie Korridore.
 * - Zwischen zwei Gebaeuden liegt jeweils ein Korridor mit streetWidth.
 *
 * Diese Art von Szenario ist nicht an eine konkrete Stadt gebunden, aber sehr
 * gut fuer Parameterstudien: Man kann Gebaeudehoehe, Blockgroesse,
 * Strassenbreite und UAV-Anzahl kontrolliert variieren.
 */
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
            building->SetNFloors(std::max<uint32_t>(1, static_cast<uint32_t>(urban.buildingHeight / 3.0)));
            buildings.push_back(building);
        }
    }

    return buildings;
}

/*
 * Schreibt die Gebaeudegeometrie in eine CSV-Datei.
 *
 * Das ist fuer die Dokumentation wichtig: Wenn spaeter nur noch Ergebnis-CSVs
 * vorhanden sind, kann ueber diese Datei rekonstruiert werden, welche
 * Gebaeudebloecke im Szenario standen.
 */
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

/*
 * Erzeugt die textbasierte Nutzlast eines Positionsupdates.
 *
 * Format:
 *   senderId sequence sendTimeSeconds x y z
 *
 * Das Format ist bewusst einfach lesbar. Fuer spaetere groessere Simulationen
 * koennte man einen eigenen ns-3 Header definieren, aber fuer diese Phase ist
 * Nachvollziehbarkeit wichtiger als maximale Effizienz.
 */
std::string
BuildPositionMessage(uint32_t senderId, uint32_t sequence, const Vector& position)
{
    std::ostringstream message;
    message << senderId << ' ' << sequence << ' ' << Simulator::Now().GetSeconds() << ' '
            << position.x << ' ' << position.y << ' ' << position.z;
    return message.str();
}

/*
 * Liest ein Positionsupdate aus der textbasierten Nutzlast.
 *
 * Rueckgabe:
 *   true, wenn alle Felder vorhanden und lesbar sind.
 *   false, wenn das Paket nicht dem erwarteten Format entspricht.
 */
bool
ParsePositionMessage(const std::string& message,
                     uint32_t& senderId,
                     uint32_t& sequence,
                     double& sendTimeSeconds,
                     Vector& position)
{
    std::istringstream input(message);
    input >> senderId >> sequence >> sendTimeSeconds >> position.x >> position.y >> position.z;
    return !input.fail();
}

/*
 * Aktualisiert Min/Max/Summe der Latenzwerte.
 *
 * Die Latenz ist weiterhin nuetzlich, weil sie zeigt, wie schnell erfolgreich
 * empfangene Pakete sind. AoI ergaenzt diese Sicht um verlorene oder fehlende
 * Updates.
 */
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

/*
 * Callback fuer eingehende UDP-Positionsupdates.
 *
 * Fuer AoI ist die wichtigste Operation in dieser Funktion:
 *
 *   g_lastGenerationTime[receiverId][senderId] = sendTimeSeconds;
 *
 * Damit ersetzt ein neueres Update die bisher bekannte Information. Bei jedem
 * spaeteren AoI-Sample kann daraus berechnet werden, wie alt diese Information
 * inzwischen ist.
 */
void
ReceivePositionUpdate(uint32_t receiverId, Ptr<Socket> socket)
{
    Address sourceAddress;
    Ptr<Packet> packet;

    while ((packet = socket->RecvFrom(sourceAddress)))
    {
        std::string payload(packet->GetSize(), '\0');
        packet->CopyData(reinterpret_cast<uint8_t*>(&payload[0]), packet->GetSize());

        uint32_t senderId;
        uint32_t sequence;
        double sendTimeSeconds;
        Vector position;
        if (!ParsePositionMessage(payload, senderId, sequence, sendTimeSeconds, position))
        {
            NS_LOG_WARN("Ignoring malformed position update: " << payload);
            continue;
        }

        // Ein UAV kennt seine eigene Position. Fuer Schwarm-Situational-
        // Awareness zaehlen hier nur Informationen ueber andere UAVs.
        if (senderId == receiverId)
        {
            continue;
        }

        const double nowSeconds = Simulator::Now().GetSeconds();
        const double latencyMs = (nowSeconds - sendTimeSeconds) * 1000.0;

        RecordLatency(latencyMs);
        g_lastGenerationTime[receiverId][senderId] = sendTimeSeconds;
        g_knownInformation[receiverId][senderId] = true;

        if (g_updateMetrics.is_open())
        {
            g_updateMetrics << std::fixed << std::setprecision(6) << nowSeconds << ','
                            << sendTimeSeconds << ',' << senderId << ',' << receiverId << ','
                            << sequence << ',' << latencyMs << ',' << position.x << ','
                            << position.y << ',' << position.z << '\n';
        }

        if (g_verbose)
        {
            std::cout << "t=" << nowSeconds << "s receiver=" << receiverId
                      << " sender=" << senderId << " seq=" << sequence
                      << " latency=" << latencyMs << "ms" << std::endl;
        }
    }
}

/*
 * Sampelt die Age of Information fuer alle gerichteten UAV-Paare.
 *
 * Gerichtetes Paar bedeutet:
 *   receiver A kennt Information ueber sender B.
 *
 * A->B und B->A werden getrennt betrachtet, weil Funkempfang und Paketverlust
 * in einer Simulation asymmetrisch sein koennen.
 */
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

/*
 * Sendet ein Positionsupdate eines UAVs und plant bei Bedarf das naechste
 * Update desselben UAVs.
 */
void
SendPositionUpdate(NodeContainer uavs, uint32_t senderId, uint32_t remainingUpdates)
{
    Ptr<MobilityModel> mobility = uavs.Get(senderId)->GetObject<MobilityModel>();
    Vector position = mobility->GetPosition();

    const uint32_t sequence = g_sequenceNumbers[senderId]++;
    std::string message = BuildPositionMessage(senderId, sequence, position);
    Ptr<Packet> packet =
        Create<Packet>(reinterpret_cast<const uint8_t*>(message.data()), message.size());

    g_sendSockets[senderId]->Send(packet);
    g_packetStats.packetsSent++;

    if (remainingUpdates > 1)
    {
        Simulator::Schedule(g_updateInterval,
                            &SendPositionUpdate,
                            uavs,
                            senderId,
                            remainingUpdates - 1);
    }
}

/*
 * Erzeugt deterministische Startpositionen entlang urbaner Strassenkorridore.
 *
 * Im freien-Feld-Basisszenario standen UAVs auf einem einfachen 2D-Gitter.
 * Hier sollen sie dagegen auf Strassenachsen verteilt werden. Dafuer werden
 * erst alle horizontalen und vertikalen Korridorlinien erzeugt. Anschliessend
 * werden die UAVs abwechselnd auf diese Linien gelegt.
 *
 * spacingMeters steuert hier nicht den Gebaeudeabstand, sondern den Abstand
 * zwischen UAVs entlang der ausgewaehlten Korridorachse. Damit bleibt die
 * Bedeutung des Parameters fuer Experimentreihen erhalten: mehr spacing heisst
 * groessere UAV-Abstaende.
 */
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

    /*
     * Die Gebaeude beginnen bei streetWidth / 2. Dadurch sind die Randstrassen
     * halb so breit wie die inneren Strassen. Fuer die UAV-Positionen nutzen
     * wir jeweils die Mitte der freien Korridore.
     */
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

    /*
     * Erst werden Kandidaten entlang horizontaler Strassen erzeugt, danach
     * entlang vertikaler Strassen. Doppelte Kreuzungspunkte werden vermieden.
     * Dadurch kann kein UAV versehentlich exakt auf demselben Punkt starten.
     */
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

        /*
         * Bei sehr grossen UAV-Zahlen koennen mehr UAVs angefragt werden, als
         * eindeutige Korridorpunkte vorhanden sind. Statt abzubrechen,
         * verschieben wir weitere Runden minimal entlang der x-Achse. Das
         * haelt die Simulation lauffaehig und macht den Effekt sichtbar.
         */
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

/*
 * Aktualisiert fuer jedes UAV die Information, ob es innerhalb oder ausserhalb
 * eines Gebaeudes liegt.
 *
 * Das HybridBuildingsPropagationLossModel erwartet, dass jedes MobilityModel
 * ein MobilityBuildingInfo-Objekt besitzt. BuildingsHelper::Install fuegt
 * dieses Objekt hinzu. MakeConsistent ordnet danach die aktuelle Position
 * einem Gebaeude oder dem Outdoor-Bereich zu.
 */
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
    uint32_t numUavs = 5;
    double simTimeSeconds = 10.0;
    double updateIntervalSeconds = 1.0;
    double aoiSampleIntervalSeconds = 0.1;
    double spacingMeters = 100.0;
    double altitudeMeters = 80.0;
    double txPowerDbm = 16.0;
    double channelFrequencyHz = 5.0e9;
    bool enablePcap = false;
    std::string updateMetricsFile = "uav-wifi-aoi-updates.csv";
    std::string aoiMetricsFile = "uav-wifi-aoi-samples.csv";
    UrbanScenarioConfig urban;
    std::string phyMode = "DsssRate11Mbps";

    /*
     * Parameter fuer Experimentreihen.
     *
     * Beispiel:
     *   ./ns3 run "uav-urban-wifi-aoi --numUavs=20 --spacing=100 --simTime=30"
     *
     * aoiSampleInterval steuert, wie fein die AoI-Zeitreihe abgetastet wird.
     * Kleine Werte liefern genauere Kurven, erzeugen aber groessere CSVs.
     */
    CommandLine cmd(__FILE__);
    cmd.AddValue("numUavs", "Number of UAV nodes", numUavs);
    cmd.AddValue("simTime", "Simulation time in seconds", simTimeSeconds);
    cmd.AddValue("updateInterval",
                 "Seconds between two position updates per UAV",
                 updateIntervalSeconds);
    cmd.AddValue("aoiSampleInterval",
                 "Seconds between two AoI samples",
                 aoiSampleIntervalSeconds);
    cmd.AddValue("spacing", "Grid spacing between UAVs in meters", spacingMeters);
    cmd.AddValue("altitude", "UAV altitude in meters", altitudeMeters);
    cmd.AddValue("txPower", "Wi-Fi transmit power in dBm", txPowerDbm);
    cmd.AddValue("frequency", "Channel frequency in Hz for the urban propagation model", channelFrequencyHz);
    cmd.AddValue("blocksX", "Number of building blocks along the x axis", urban.blocksX);
    cmd.AddValue("blocksY", "Number of building blocks along the y axis", urban.blocksY);
    cmd.AddValue("buildingLengthX", "Building footprint length along x in meters", urban.buildingLengthX);
    cmd.AddValue("buildingLengthY", "Building footprint length along y in meters", urban.buildingLengthY);
    cmd.AddValue("streetWidth", "Width of streets between buildings in meters", urban.streetWidth);
    cmd.AddValue("buildingHeight", "Building height in meters", urban.buildingHeight);
    cmd.AddValue("updateMetricsFile", "CSV file for received position updates", updateMetricsFile);
    cmd.AddValue("aoiMetricsFile", "CSV file for AoI samples", aoiMetricsFile);
    cmd.AddValue("buildingMetricsFile", "CSV file for generated building geometry", urban.buildingMetricsFile);
    cmd.AddValue("enablePcap", "Enable Wi-Fi pcap tracing", enablePcap);
    cmd.AddValue("verbose", "Print every received position update", g_verbose);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(numUavs < 2, "numUavs must be at least 2");
    NS_ABORT_MSG_IF(simTimeSeconds <= 0.0, "simTime must be positive");
    NS_ABORT_MSG_IF(updateIntervalSeconds <= 0.0, "updateInterval must be positive");
    NS_ABORT_MSG_IF(aoiSampleIntervalSeconds <= 0.0, "aoiSampleInterval must be positive");
    NS_ABORT_MSG_IF(urban.blocksX == 0 || urban.blocksY == 0, "blocksX and blocksY must be positive");
    NS_ABORT_MSG_IF(urban.buildingLengthX <= 0.0 || urban.buildingLengthY <= 0.0,
                    "building lengths must be positive");
    NS_ABORT_MSG_IF(urban.streetWidth <= 0.0, "streetWidth must be positive");
    NS_ABORT_MSG_IF(urban.buildingHeight <= 0.0, "buildingHeight must be positive");

    g_updateInterval = Seconds(updateIntervalSeconds);
    g_aoiSampleInterval = Seconds(aoiSampleIntervalSeconds);
    g_stopTime = Seconds(simTimeSeconds + 1.0);
    g_updatesPerUav = static_cast<uint32_t>(std::floor(simTimeSeconds / updateIntervalSeconds));
    NS_ABORT_MSG_IF(g_updatesPerUav == 0, "simTime must allow at least one update per UAV");

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

    InternetStackHelper internet;
    internet.Install(uavs);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.0.0", "255.255.255.0");
    ipv4.Assign(devices);

    TypeId udpFactory = TypeId::LookupByName("ns3::UdpSocketFactory");
    g_sendSockets.resize(numUavs);
    g_sequenceNumbers.assign(numUavs, 0);
    g_lastGenerationTime.assign(numUavs, std::vector<double>(numUavs, 0.0));
    g_knownInformation.assign(numUavs, std::vector<bool>(numUavs, false));

    for (uint32_t i = 0; i < numUavs; ++i)
    {
        Ptr<Socket> receiveSocket = Socket::CreateSocket(uavs.Get(i), udpFactory);
        receiveSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UavPositionPort));
        receiveSocket->SetRecvCallback(MakeBoundCallback(&ReceivePositionUpdate, i));

        Ptr<Socket> sendSocket = Socket::CreateSocket(uavs.Get(i), udpFactory);
        sendSocket->SetAllowBroadcast(true);
        sendSocket->Connect(InetSocketAddress(Ipv4Address("255.255.255.255"), UavPositionPort));
        g_sendSockets[i] = sendSocket;
    }

    g_updateMetrics.open(updateMetricsFile);
    g_updateMetrics << "receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,x_m,y_m,z_m\n";

    g_aoiMetrics.open(aoiMetricsFile);
    g_aoiMetrics << "time_s,receiver_id,sender_id,known,aoi_s\n";

    if (enablePcap)
    {
        wifiPhy.EnablePcap("uav-urban-wifi-aoi", devices);
    }

    PrintUrbanScenario(urban, buildings.size());
    PrintUavPositions(uavs);

    // AoI wird ab t=0 gesampelt. Anfangs ist known=0 fuer alle Paare, weil
    // noch kein UAV ein Positionsupdate eines anderen UAVs erhalten hat.
    Simulator::Schedule(Seconds(0.0), &SampleAoi, uavs);

    // Leicht versetzte Startzeiten vermeiden, dass alle UAVs exakt in
    // demselben Simulationszeitpunkt senden.
    for (uint32_t i = 0; i < numUavs; ++i)
    {
        Simulator::Schedule(Seconds(1.0 + i * 0.01), &SendPositionUpdate, uavs, i, g_updatesPerUav);
    }

    Simulator::Stop(g_stopTime);
    Simulator::Run();
    Simulator::Destroy();

    g_updateMetrics.close();
    g_aoiMetrics.close();

    const uint32_t expectedReceives = g_packetStats.packetsSent * (numUavs - 1);
    const double deliveryRatio = expectedReceives > 0
                                     ? static_cast<double>(g_packetStats.packetsReceived) /
                                           static_cast<double>(expectedReceives)
                                     : 0.0;
    const double averageLatencyMs = g_packetStats.packetsReceived > 0
                                        ? g_packetStats.latencySumMs /
                                              static_cast<double>(g_packetStats.packetsReceived)
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
    std::cout << "Architecture: Urban Wi-Fi ad hoc UDP broadcast with AoI sampling" << std::endl;
    std::cout << "UAVs: " << numUavs << std::endl;
    std::cout << "Buildings: " << buildings.size() << std::endl;
    std::cout << "Street width: " << urban.streetWidth << " m" << std::endl;
    std::cout << "Building height: " << urban.buildingHeight << " m" << std::endl;
    std::cout << "Sent packets: " << g_packetStats.packetsSent << std::endl;
    std::cout << "Received packets: " << g_packetStats.packetsReceived << " / " << expectedReceives
              << std::endl;
    std::cout << "Delivery ratio: " << deliveryRatio << std::endl;
    std::cout << "Average latency: " << averageLatencyMs << " ms" << std::endl;
    std::cout << "Min latency: " << g_packetStats.latencyMinMs << " ms" << std::endl;
    std::cout << "Max latency: " << g_packetStats.latencyMaxMs << " ms" << std::endl;
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
