#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

/*
 * UAV Wi-Fi baseline
 * ------------------
 *
 * Diese Datei ist die erste "saubere" Ausbaustufe nach uav-test.cc.
 * Sie soll als nachvollziehbare Grundlage fuer die Bachelorarbeit dienen:
 *
 * - Mehrere UAVs werden als ns-3 Nodes modelliert.
 * - Die UAVs befinden sich auf einem einfachen 2D-Gitter in gleicher Hoehe.
 * - Jedes UAV besitzt ein Wi-Fi-Interface im Ad-hoc-Modus.
 * - Jedes UAV sendet periodisch seine aktuelle Position per UDP-Broadcast.
 * - Alle anderen UAVs empfangen diese Positionsupdates und schreiben
 *   Messwerte in eine CSV-Datei.
 *
 * Warum Wi-Fi Ad-hoc?
 * Fuer UAV-Schwaerme ist Wi-Fi/WLAN ein sinnvoller erster Architekturtyp,
 * weil damit direkte Kommunikation ohne Basisstation modelliert werden kann.
 * Diese Datei ist damit zugleich eine Grundlage fuer spaetere Varianten:
 *
 * - Broadcast-Ansatz: wie hier, alle UAVs senden an alle Nachbarn.
 * - Mesh-Ansatz: spaeter mit Routing-Protokollen wie OLSR oder AODV.
 * - Vergleichsbasis fuer LTE/5G: dort laeuft Kommunikation ueber Infrastruktur.
 *
 * Wichtig: Dieses Modell ist bewusst noch vereinfacht. Die UAVs bewegen sich
 * nicht, es gibt keine Gebaeudeabschattung und alle Positionsupdates enthalten
 * nur Sender-ID, Sequenznummer, Sendezeit und Position. Genau dadurch ist die
 * Datei aber gut geeignet, um die Messkette zuerst stabil aufzubauen.
 */
NS_LOG_COMPONENT_DEFINE("UavWifiBaseline");

namespace
{

// Alle UAVs verwenden denselben UDP-Port fuer Positionsupdates.
// Das macht den Broadcast-Charakter sichtbar: jedes UAV bindet einen
// Empfangs-Socket auf diesen Port, und jedes UAV sendet an die Broadcast-Adresse.
constexpr uint16_t UavPositionPort = 9000;

/*
 * Einfache Sammelstruktur fuer die wichtigsten Simulationsmetriken.
 *
 * packetsSent:
 *   Anzahl der von allen UAVs erzeugten Positionsupdates.
 *
 * packetsReceived:
 *   Anzahl der erfolgreich empfangenen Positionsupdates. Eigene Broadcasts
 *   werden ignoriert, weil ein UAV seine eigene Position bereits kennt.
 *
 * latency*:
 *   Ende-zu-Ende-Verzoegerung zwischen Sendetermin und Empfangstermin.
 *   Diese Werte sind fuer Forschungsfrage 1 direkt relevant.
 */
struct SimulationStats
{
    uint32_t packetsSent{0};
    uint32_t packetsReceived{0};
    uint64_t appBytesSent{0};
    uint64_t appBytesReceived{0};
    double latencySumMs{0.0};
    double latencyMinMs{0.0};
    double latencyMaxMs{0.0};
};

// Die Scratch-Datei nutzt globale Variablen, um den Code fuer den Einstieg
// kompakt zu halten. Fuer ein groesseres Framework koennte daraus spaeter
// eine eigene Experiment-Klasse werden.
std::vector<Ptr<Socket>> g_sendSockets;
std::vector<uint32_t> g_sequenceNumbers;
SimulationStats g_stats;
std::ofstream g_metrics;
Time g_updateInterval;
uint32_t g_updatesPerUav{0};
bool g_verbose{false};

/*
 * Erzeugt die Nutzdaten eines Positionsupdates.
 *
 * Format:
 *   senderId sequence sendTimeSeconds x y z
 *
 * Das Format ist absichtlich textbasiert. Binaere Header waeren effizienter,
 * aber Text ist fuer den Anfang leichter in CSVs, Logs und Debug-Ausgaben
 * nachzuvollziehen. Fuer die spaetere wissenschaftliche Auswertung ist diese
 * Transparenz wichtiger als ein paar Byte Overhead.
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
 * Zerlegt eine empfangene Positionsnachricht wieder in ihre Felder.
 *
 * Rueckgabe:
 *   true, wenn alle erwarteten Felder gelesen werden konnten.
 *   false, wenn das Paket nicht dem erwarteten Textformat entspricht.
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
 * Callback fuer eingehende UDP-Pakete.
 *
 * ns-3 ruft diese Funktion auf, sobald am Socket eines UAVs ein Paket
 * ankommt. Die Funktion liest alle aktuell verfuegbaren Pakete aus dem
 * Socket, interpretiert sie als Positionsupdates und aktualisiert die
 * Metriken.
 *
 * receiverId wird ueber MakeBoundCallback fest an den jeweiligen Socket
 * gebunden. Dadurch weiss die Funktion, welches UAV gerade empfaengt.
 */
void
ReceivePositionUpdate(uint32_t receiverId, Ptr<Socket> socket)
{
    Address sourceAddress;
    Ptr<Packet> packet;

    while ((packet = socket->RecvFrom(sourceAddress)))
    {
        // ns-3 Pakete enthalten rohe Bytes. Fuer diese Baseline wandeln wir
        // die Bytes wieder in den oben definierten Textstring um.
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

        // Bei Broadcast kann je nach Stack/Modell auch das eigene Paket
        // sichtbar werden. Fuer Positionsaktualitaet zwischen UAVs zaehlen
        // nur Nachrichten von anderen UAVs.
        if (senderId == receiverId)
        {
            continue;
        }

        // Die Latenz wird als Differenz zwischen aktueller Simulationszeit
        // und im Paket gespeicherter Sendezeit berechnet. Wir nutzen Sekunden
        // als double und rechnen selbst in Millisekunden um, damit auch
        // Sub-Millisekunden-Werte sichtbar bleiben.
        const double latencyMs = (Simulator::Now().GetSeconds() - sendTimeSeconds) * 1000.0;
        g_stats.packetsReceived++;
        g_stats.appBytesReceived += packetSize;
        g_stats.latencySumMs += latencyMs;

        // Min/Max muessen beim ersten empfangenen Paket initialisiert werden.
        // Danach koennen sie normal fortgeschrieben werden.
        if (g_stats.packetsReceived == 1)
        {
            g_stats.latencyMinMs = latencyMs;
            g_stats.latencyMaxMs = latencyMs;
        }
        else
        {
            g_stats.latencyMinMs = std::min(g_stats.latencyMinMs, latencyMs);
            g_stats.latencyMaxMs = std::max(g_stats.latencyMaxMs, latencyMs);
        }

        // Jede empfangene Positionsinformation wird als einzelne CSV-Zeile
        // gespeichert. Das ist die Rohdatenbasis fuer spaetere Auswertungen
        // in Python, R, Tabellenkalkulation oder Plot-Skripten.
        if (g_metrics.is_open())
        {
            g_metrics << std::fixed << std::setprecision(6) << Simulator::Now().GetSeconds()
                      << ',' << senderId << ',' << receiverId << ',' << sequence << ','
                      << latencyMs << ',' << packetSize << ',' << position.x << ','
                      << position.y << ',' << position.z << '\n';
        }

        // Verbose-Ausgabe ist nuetzlich beim Debuggen, wuerde aber bei vielen
        // UAVs sehr schnell unuebersichtlich werden. Deshalb ist sie optional.
        if (g_verbose)
        {
            std::cout << "t=" << Simulator::Now().GetSeconds() << "s UAV " << receiverId
                      << " received update " << sequence << " from UAV " << senderId
                      << " latency=" << latencyMs << "ms" << std::endl;
        }
    }
}

/*
 * Sendet ein Positionsupdate eines bestimmten UAVs.
 *
 * Die Funktion plant sich selbst erneut ein, solange remainingUpdates > 1 ist.
 * Dadurch entsteht pro UAV eine periodische Sendereihe mit dem Abstand
 * g_updateInterval.
 */
void
SendPositionUpdate(NodeContainer uavs, uint32_t senderId, uint32_t remainingUpdates)
{
    // Die aktuelle Position kommt aus dem MobilityModel. Momentan ist das
    // ConstantPositionMobilityModel; spaeter kann hier ein Bewegungsmodell
    // eingesetzt werden, ohne das Nachrichtenformat zu aendern.
    Ptr<MobilityModel> mobility = uavs.Get(senderId)->GetObject<MobilityModel>();
    Vector position = mobility->GetPosition();

    // Jede UAV-eigene Sequenznummer steigt pro gesendetem Update um eins.
    // Das hilft spaeter, Paketverluste und veraltete Positionsinformationen
    // eindeutig zu erkennen.
    const uint32_t sequence = g_sequenceNumbers[senderId]++;
    std::string message = BuildPositionMessage(senderId, sequence, position);
    Ptr<Packet> packet =
        Create<Packet>(reinterpret_cast<const uint8_t*>(message.data()), message.size());

    // Der Socket wurde vorher mit der IPv4-Broadcast-Adresse verbunden.
    // Ein Send() reicht deshalb aus, um das Update an alle erreichbaren UAVs
    // im selben Ad-hoc-Funknetz auszusenden.
    g_sendSockets[senderId]->Send(packet);
    g_stats.packetsSent++;
    g_stats.appBytesSent += message.size();

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
 * Erzeugt Startpositionen auf einem quadratischen Gitter.
 *
 * Beispiel fuer 5 UAVs und 100 m Abstand:
 *   UAV 0: (0,   0, 80)
 *   UAV 1: (100, 0, 80)
 *   UAV 2: (200, 0, 80)
 *   UAV 3: (0, 100, 80)
 *   UAV 4: (100, 100, 80)
 *
 * Diese deterministische Platzierung ist fuer die erste Baseline besser als
 * Zufallspositionen, weil Ergebnisse reproduzierbar bleiben.
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

// Gibt die gewaehlten Startpositionen aus. Das ist eine einfache Plausibilitaets-
// kontrolle, bevor die eigentliche Netzwerksimulation startet.
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
    // Standardparameter fuer einen kleinen, schnell laufenden Test.
    // Alle Werte koennen ueber die Kommandozeile ueberschrieben werden.
    uint32_t numUavs = 5;
    double simTimeSeconds = 10.0;
    double updateIntervalSeconds = 1.0;
    double spacingMeters = 100.0;
    double altitudeMeters = 80.0;
    double txPowerDbm = 16.0;
    double channelFrequencyHz = 2.4e9;
    uint64_t rngRun = 1;
    bool enablePcap = false;
    std::string metricsFile = "uav-wifi-baseline-metrics.csv";
    std::string phyMode = "DsssRate11Mbps";

    /*
     * Kommandozeilenparameter machen die Datei fuer Experimentreihen nutzbar.
     *
     * Beispiele:
     *   ./ns3 run "uav-wifi-baseline --numUavs=20 --simTime=30"
     *   ./ns3 run "uav-wifi-baseline --spacing=200 --txPower=10"
     *
     * Damit koennen spaeter Skalierungsexperimente durchgefuehrt werden,
     * ohne den C++-Code fuer jede Konfiguration anzufassen.
     */
    CommandLine cmd(__FILE__);
    cmd.AddValue("numUavs", "Number of UAV nodes", numUavs);
    cmd.AddValue("simTime", "Simulation time in seconds", simTimeSeconds);
    cmd.AddValue("updateInterval",
                 "Seconds between two position updates per UAV",
                 updateIntervalSeconds);
    cmd.AddValue("spacing", "Grid spacing between UAVs in meters", spacingMeters);
    cmd.AddValue("altitude", "UAV altitude in meters", altitudeMeters);
    cmd.AddValue("txPower", "Wi-Fi transmit power in dBm", txPowerDbm);
    cmd.AddValue("frequency",
                 "Carrier frequency in Hz for the Friis propagation model",
                 channelFrequencyHz);
    cmd.AddValue("rngRun", "ns-3 RNG run number for reproducible repetitions", rngRun);
    cmd.AddValue("metricsFile", "CSV file for received position updates", metricsFile);
    cmd.AddValue("enablePcap", "Enable Wi-Fi pcap tracing", enablePcap);
    cmd.AddValue("verbose", "Print every received position update", g_verbose);
    cmd.Parse(argc, argv);

    // Fruehe Plausibilitaetspruefungen verhindern schwer verstaendliche
    // Folgefehler, z.B. Divisionen durch null oder ein Netzwerk mit nur
    // einem Teilnehmer.
    NS_ABORT_MSG_IF(numUavs < 2, "numUavs must be at least 2");
    NS_ABORT_MSG_IF(simTimeSeconds <= 0.0, "simTime must be positive");
    NS_ABORT_MSG_IF(updateIntervalSeconds <= 0.0, "updateInterval must be positive");
    NS_ABORT_MSG_IF(rngRun == 0, "rngRun must be positive");

    RngSeedManager::SetRun(rngRun);

    // Anzahl der Positionsupdates pro UAV. floor() bedeutet:
    // Bei simTime=10 und interval=3 sendet jedes UAV 3 Updates.
    g_updateInterval = Seconds(updateIntervalSeconds);
    g_updatesPerUav = static_cast<uint32_t>(std::floor(simTimeSeconds / updateIntervalSeconds));
    NS_ABORT_MSG_IF(g_updatesPerUav == 0, "simTime must allow at least one update per UAV");

    // Broadcast-Frames sollen dieselbe Datenrate nutzen wie Unicast-Frames.
    // Ohne diese Einstellung koennen Broadcasts je nach Wi-Fi-Standard auf
    // eine andere Standardrate fallen, was Vergleiche schwerer macht.
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    // Jeder Node repraesentiert ein UAV. ns-3 trennt bewusst zwischen Node,
    // MobilityModel, NetDevice und Protokollstack; die folgenden Abschnitte
    // bauen diese Schichten Schritt fuer Schritt zusammen.
    NodeContainer uavs;
    uavs.Create(numUavs);

    // Mobility: In dieser Baseline bleiben die UAVs statisch.
    // Das isoliert zunaechst die Kommunikationsarchitektur. Bewegung kann
    // spaeter ergaenzt werden, wenn die Messkette stabil ist.
    MobilityHelper mobility;
    mobility.SetPositionAllocator(CreateGridPositionAllocator(numUavs, spacingMeters, altitudeMeters));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(uavs);

    /*
     * Wi-Fi-Konfiguration:
     *
     * WIFI_STANDARD_80211b und DsssRate11Mbps sind einfache, klassische
     * Einstellungen fuer Ad-hoc-Beispiele in ns-3. Fuer die Bachelorarbeit
     * ist wichtiger, dass diese Parameter dokumentiert und konstant gehalten
     * werden, damit spaetere Architekturen fair verglichen werden koennen.
     */
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue(phyMode),
                                 "ControlMode",
                                 StringValue(phyMode));

    // Funkkanal: ConstantSpeedPropagationDelayModel modelliert die
    // Ausbreitungsverzoegerung mit Lichtgeschwindigkeit. Friis ist ein
    // einfaches Freiraum-Pfadverlustmodell. Die Frequenz wird explizit auf
    // 2.4 GHz gesetzt, damit 802.11b, DsssRate11Mbps und Pfadverlustmodell
    // zusammenpassen. Urbane Abschattung durch Gebaeude ist hier noch nicht
    // enthalten und waere ein spaeterer Modellschritt.
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                   "Frequency",
                                   DoubleValue(channelFrequencyHz));

    // Physikalische Wi-Fi-Schicht. TxPowerStart und TxPowerEnd werden auf
    // denselben Wert gesetzt, damit jedes Paket mit konstanter Sendeleistung
    // abgestrahlt wird.
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.Set("TxPowerStart", DoubleValue(txPowerDbm));
    wifiPhy.Set("TxPowerEnd", DoubleValue(txPowerDbm));
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

    // AdhocWifiMac bedeutet: Es gibt keinen Access Point und keine zentrale
    // Infrastruktur. Alle UAVs teilen sich denselben Funkkanal direkt.
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    // Installiert je ein Wi-Fi-NetDevice auf jedem UAV-Node.
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, uavs);

    // InternetStackHelper installiert IPv4, UDP/TCP und Routing-Basislogik.
    // Auch fuer reine UDP-Broadcasts brauchen wir diesen Stack, damit die
    // ns-3 Sockets IP-Pakete erzeugen und empfangen koennen.
    InternetStackHelper internet;
    internet.Install(uavs);

    // Alle Wi-Fi-Interfaces bekommen Adressen im selben Subnetz.
    // Das passt zum Broadcast-Experiment, weil alle UAVs im selben lokalen
    // Ad-hoc-Netz liegen.
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.0.0", "255.255.255.0");
    ipv4.Assign(devices);

    /*
     * Socket-Aufbau:
     *
     * Pro UAV gibt es zwei Sockets:
     * - receiveSocket: lauscht auf allen lokalen IPv4-Adressen am UAV-Port.
     * - sendSocket: sendet an 255.255.255.255, also IPv4-Broadcast.
     *
     * Dadurch entsteht eine einfache All-to-All-Kommunikation ohne Routing.
     */
    TypeId udpFactory = TypeId::LookupByName("ns3::UdpSocketFactory");
    g_sendSockets.resize(numUavs);
    g_sequenceNumbers.assign(numUavs, 0);

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

    // CSV-Datei fuer Rohdaten. Jede Zeile steht fuer ein erfolgreich
    // empfangenes Positionsupdate bei einem bestimmten Empfaenger.
    g_metrics.open(metricsFile);
    g_metrics << "time_s,sender_id,receiver_id,sequence,latency_ms,payload_bytes,x_m,y_m,z_m\n";

    // PCAP-Traces sind optional, weil sie viele Dateien erzeugen koennen.
    // Sie sind hilfreich, wenn spaeter mit Wireshark oder tcpdump einzelne
    // Frames untersucht werden sollen.
    if (enablePcap)
    {
        wifiPhy.EnablePcap("uav-wifi-baseline", devices);
    }

    PrintUavPositions(uavs);

    // Startzeiten werden leicht gegeneinander versetzt. Wenn alle UAVs exakt
    // zur selben Simulationszeit senden, entstehen kuenstliche Kollisionen,
    // die mehr ueber die Synchronisation des Testfalls als ueber die
    // Architektur aussagen wuerden.
    for (uint32_t i = 0; i < numUavs; ++i)
    {
        Simulator::Schedule(Seconds(1.0 + i * 0.01), &SendPositionUpdate, uavs, i, g_updatesPerUav);
    }

    // Eine zusaetzliche Sekunde nach simTime gibt dem letzten geplanten Update
    // Zeit, den Funkkanal zu durchlaufen und beim Empfaenger anzukommen.
    Simulator::Stop(Seconds(simTimeSeconds + 1.0));
    Simulator::Run();
    Simulator::Destroy();

    g_metrics.close();

    /*
     * Zusammenfassung:
     *
     * Bei Broadcast sollte jedes gesendete Paket von allen anderen UAVs
     * empfangen werden. Daher ist die erwartete Empfangszahl:
     *
     *   sentPackets * (numUavs - 1)
     *
     * Die Delivery Ratio ist eine erste Kenngroesse fuer Paketverlust und
     * Skalierung. Latenzwerte beschreiben die Aktualitaet der Positionsdaten.
     */
    const uint32_t expectedReceives = g_stats.packetsSent * (numUavs - 1);
    const double deliveryRatio =
        expectedReceives > 0 ? static_cast<double>(g_stats.packetsReceived) / expectedReceives : 0.0;
    const double averageLatencyMs =
        g_stats.packetsReceived > 0 ? g_stats.latencySumMs / g_stats.packetsReceived : 0.0;

    std::cout << "\nSimulation summary" << std::endl;
    std::cout << "Architecture: Wi-Fi ad hoc UDP broadcast" << std::endl;
    std::cout << "UAVs: " << numUavs << std::endl;
    std::cout << "Sent packets: " << g_stats.packetsSent << std::endl;
    std::cout << "Received packets: " << g_stats.packetsReceived << " / " << expectedReceives
              << std::endl;
    std::cout << "Delivery ratio: " << deliveryRatio << std::endl;
    std::cout << "Application bytes sent: " << g_stats.appBytesSent << std::endl;
    std::cout << "Application bytes received: " << g_stats.appBytesReceived << std::endl;
    std::cout << "Average latency: " << averageLatencyMs << " ms" << std::endl;
    std::cout << "Min latency: " << g_stats.latencyMinMs << " ms" << std::endl;
    std::cout << "Max latency: " << g_stats.latencyMaxMs << " ms" << std::endl;
    std::cout << "Metrics CSV: " << metricsFile << std::endl;

    return 0;
}
