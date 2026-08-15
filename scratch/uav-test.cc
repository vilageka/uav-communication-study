#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int
main(int argc, char *argv[]) {
    NodeContainer uavs;
    uavs.Create(5);

    Ptr<ListPositionAllocator> positionAlloc =
        CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 80.0));
    positionAlloc->Add(Vector(100.0, 0.0, 80.0));
    positionAlloc->Add(Vector(200.0, 0.0, 80.0));
    positionAlloc->Add(Vector(300.0, 0.0, 80.0));
    positionAlloc->Add(Vector(400.0, 0.0, 80.0));

    MobilityHelper mobility;

    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel(
        "ns3::ConstantPositionMobilityModel");

    mobility.Install(uavs);



    Simulator::Run();
    Simulator::Destroy();
    std::cout << "Sim laeuft"
    << uavs.GetN()
    << std::endl;


    for (uint32_t i = 0; i < uavs.GetN(); i++) {
        Ptr<MobilityModel> mobility = uavs.Get(i) -> GetObject <MobilityModel>();
        Vector pos = mobility->GetPosition();

        std::cout << "UAV " << i
                  << " : ("
                  << pos.x << ","
                  << pos.y << ","
                  << pos.z << ")"
                  << std::endl;
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
