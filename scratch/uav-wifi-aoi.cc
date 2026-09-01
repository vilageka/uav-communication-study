#include "ns3/core-module.h"
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
 * UAV Wi-Fi AoI experiment
 * ------------------------
 *
 * Diese Datei erweitert uav-wifi-baseline.cc um Age of Information (AoI).
 * Die Basis bleibt gleich:
 *
 * - UAVs werden als ns-3 Nodes modelliert.
 * - Jedes UAV hat eine statische Position auf einem 2D-Gitter.
 * - Alle UAVs kommunizieren ueber Wi-Fi Ad-hoc.
 * - Positionsupdates werden periodisch per UDP-Broadcast verschickt.
 *
 * Neu in dieser Version:
 *
 * - Jedes empfangene Positionsupdate enthaelt seine Erzeugungszeit.
 * - Jeder Empfaenger merkt sich fuer jeden anderen UAV den letzten bekannten
 *   Erzeugungszeitpunkt einer Positionsinformation.
 * - Die Simulation sampelt periodisch die Age of Information:
 *
 *     AoI(receiver, sender, t) = t - generationTime(lastUpdate(sender))
 *
 *   Das ist fuer die Bachelorarbeit besonders relevant, weil AoI nicht nur
 *   erfolgreiche Pakete betrachtet. Wenn Updates verloren gehen oder ein UAV
 *   nicht erreichbar ist, steigt die AoI weiter an.
 *
 * Interpretation:
 *
 * - Niedrige Latenz bedeutet: Ein empfangenes Paket war schnell.
 * - Niedrige AoI bedeutet: Die aktuell verfuegbare Positionsinformation ist
 *   frisch.
 *
 * Ein Kommunikationsansatz kann also niedrige Latenz fuer empfangene Pakete
 * haben, aber trotzdem schlechte AoI, wenn viele Updates gar nicht ankommen.
 */
NS_LOG_COMPONENT_DEFINE("UavWifiAoi");

namespace
{

// Alle UAVs nutzen denselben UDP-Port fuer Positionsupdates.
constexpr uint16_t UavPositionPort = 9000;

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
    uint64_t appBytesSent{0};
    uint64_t appBytesReceived{0};
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

Time g_updateInterval;
Time g_aoiSampleInterval;
Time g_stopTime;
uint32_t g_updatesPerUav{0};
bool g_verbose{false};

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
 * Prueft, ob ein empfangenes Update den gespeicherten Informationsstand
 * verbessern darf.
 *
 * Fuer AoI zaehlt immer die frischeste bekannte Information. Falls spaeter ein
 * aelteres Paket ankommt, darf es PDR und Latenz weiterhin beeinflussen, aber
 * es darf den AoI-Zustand nicht auf einen aelteren Erzeugungszeitpunkt
 * zuruecksetzen.
 */
bool
ShouldAcceptInformationUpdate(uint32_t receiverId, uint32_t senderId, double sendTimeSeconds)
{
    return !g_knownInformation[receiverId][senderId] ||
           sendTimeSeconds > g_lastGenerationTime[receiverId][senderId];
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
        const uint32_t packetSize = packet->GetSize();
        std::string payload(packetSize, '\0');
        packet->CopyData(reinterpret_cast<uint8_t*>(&payload[0]), packetSize);

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
                            << sequence << ',' << latencyMs << ',' << packetSize << ','
                            << position.x << ',' << position.y << ',' << position.z << '\n';
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
    g_packetStats.appBytesSent += message.size();

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
 * Erzeugt deterministische Startpositionen auf einem Gitter.
 *
 * Deterministische Positionen sind fuer reproduzierbare Messungen wichtig.
 * Zufallspositionen koennen spaeter als eigene Szenarien ergaenzt werden.
 */
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
    uint32_t numUavs = 5;
    double simTimeSeconds = 10.0;
    double updateIntervalSeconds = 1.0;
    double aoiSampleIntervalSeconds = 0.1;
    double spacingMeters = 100.0;
    double altitudeMeters = 80.0;
    double txPowerDbm = 16.0;
    uint64_t rngRun = 1;
    bool enablePcap = false;
    std::string updateMetricsFile = "uav-wifi-aoi-updates.csv";
    std::string aoiMetricsFile = "uav-wifi-aoi-samples.csv";
    std::string phyMode = "DsssRate11Mbps";

    /*
     * Parameter fuer Experimentreihen.
     *
     * Beispiel:
     *   ./ns3 run "uav-wifi-aoi --numUavs=20 --spacing=100 --simTime=30"
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
    cmd.AddValue("rngRun", "ns-3 RNG run number for reproducible repetitions", rngRun);
    cmd.AddValue("updateMetricsFile", "CSV file for received position updates", updateMetricsFile);
    cmd.AddValue("aoiMetricsFile", "CSV file for AoI samples", aoiMetricsFile);
    cmd.AddValue("enablePcap", "Enable Wi-Fi pcap tracing", enablePcap);
    cmd.AddValue("verbose", "Print every received position update", g_verbose);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(numUavs < 2, "numUavs must be at least 2");
    NS_ABORT_MSG_IF(simTimeSeconds <= 0.0, "simTime must be positive");
    NS_ABORT_MSG_IF(updateIntervalSeconds <= 0.0, "updateInterval must be positive");
    NS_ABORT_MSG_IF(aoiSampleIntervalSeconds <= 0.0, "aoiSampleInterval must be positive");
    NS_ABORT_MSG_IF(rngRun == 0, "rngRun must be positive");

    RngSeedManager::SetRun(rngRun);

    g_updateInterval = Seconds(updateIntervalSeconds);
    g_aoiSampleInterval = Seconds(aoiSampleIntervalSeconds);
    g_stopTime = Seconds(simTimeSeconds + 1.0);
    g_updatesPerUav = static_cast<uint32_t>(std::floor(simTimeSeconds / updateIntervalSeconds));
    NS_ABORT_MSG_IF(g_updatesPerUav == 0, "simTime must allow at least one update per UAV");

    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    NodeContainer uavs;
    uavs.Create(numUavs);

    MobilityHelper mobility;
    mobility.SetPositionAllocator(CreateGridPositionAllocator(numUavs, spacingMeters, altitudeMeters));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(uavs);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue(phyMode),
                                 "ControlMode",
                                 StringValue(phyMode));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

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
    g_updateMetrics << "receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,"
                    << "payload_bytes,x_m,y_m,z_m\n";

    g_aoiMetrics.open(aoiMetricsFile);
    g_aoiMetrics << "time_s,receiver_id,sender_id,known,aoi_s\n";

    if (enablePcap)
    {
        wifiPhy.EnablePcap("uav-wifi-aoi", devices);
    }

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
    std::cout << "Architecture: Wi-Fi ad hoc UDP broadcast with AoI sampling" << std::endl;
    std::cout << "UAVs: " << numUavs << std::endl;
    std::cout << "Sent packets: " << g_packetStats.packetsSent << std::endl;
    std::cout << "Received packets: " << g_packetStats.packetsReceived << " / " << expectedReceives
              << std::endl;
    std::cout << "Delivery ratio: " << deliveryRatio << std::endl;
    std::cout << "Application bytes sent: " << g_packetStats.appBytesSent << std::endl;
    std::cout << "Application bytes received: " << g_packetStats.appBytesReceived << std::endl;
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

    return 0;
}
