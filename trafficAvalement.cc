#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/gnuplot.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project");

int main(int argc, char *argv[])
{
    bool verbose = true;
    uint32_t packets = 10; // Two CSMA buses with two nodes each
    uint32_t stop = 20;
    CommandLine cmd;
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer p2pNodes; //equivalent of p2pNodesLAN1toLAN2
    p2pNodes.Create(2);

    // Creation of the two other p2p nodes 
    NodeContainer p2pNodesLAN2toLAN3; // On va creer un noeud et ajouter le noeud de p2p existant dans le LAN2
    p2pNodesLAN2toLAN3.Add(p2pNodes.Get(1));
    p2pNodesLAN2toLAN3.Create(1);

    // Creation d'une connexion p2p entre le LAN3 et le noeud seul n8 : 
    NodeContainer p2pNodesLAN3N8; // Le noeud du p2p dans le LAN3 existe deja 
    p2pNodesLAN3N8.Add(p2pNodesLAN2toLAN3.Get(1));//Ajout du noeud du LAN 3 
    p2pNodesLAN3N8.Create(1);//Ajout du noeud n8

    // Creation d'une connexion p2p entre le N8 et le LAN1 : 
    NodeContainer p2pNodesN8LAN1; // Le noeud du p2p dans le LAN3 existe deja 
    p2pNodesN8LAN1.Add(p2pNodesLAN3N8.Get(1));//Ajout du noeud N8
    p2pNodesN8LAN1.Add(p2pNodes.Get(0));//Ajout du noeud du LAN 1

   // Creation d'une connexion p2p entre le N8 et le LAN 2 : 
    NodeContainer p2pNodesN8LAN2; // Le noeud du p2p dans le LAN3 existe deja 
    p2pNodesN8LAN2.Add(p2pNodesLAN3N8.Get(1));//Ajout du noeud N8
    p2pNodesN8LAN2.Add(p2pNodes.Get(1));//Ajout du noeud du LAN 2

    NodeContainer csmaNodes1; // First CSMA bus
    csmaNodes1.Add(p2pNodes.Get(0));
    csmaNodes1.Create(2);

    NodeContainer csmaNodes2; // Second CSMA bus
    csmaNodes2.Add(p2pNodes.Get(1)); // Connect to the first node of the point-to-point link
    csmaNodes2.Create(2);

    NodeContainer csmaNodes3; // Third CSMA bus
    csmaNodes3.Add(p2pNodesLAN2toLAN3.Get(1)); // Connect to the first node of the point-to-point link
    csmaNodes3.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("15Mbps")); //Diminution du dataRate et auggmentation du Delai
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    PointToPointHelper pointToPointLAN2LAN3;
    pointToPointLAN2LAN3.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointLAN2LAN3.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices1;
    p2pDevices1 = pointToPointLAN2LAN3.Install(p2pNodesLAN2toLAN3); // duplication des noeuds a ce niveau 

    PointToPointHelper pointToPointLAN3N8;
    pointToPointLAN3N8.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointLAN3N8.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices2;
    p2pDevices2 = pointToPointLAN3N8.Install(p2pNodesLAN3N8); // duplication des noeuds a ce niveau 

    PointToPointHelper pointToPointN8LAN1;
    pointToPointN8LAN1.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointN8LAN1.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices3;
    p2pDevices3 = pointToPointN8LAN1.Install(p2pNodesN8LAN1); // duplication des noeuds a ce niveau 

    PointToPointHelper pointToPointN8LAN2;
    pointToPointN8LAN2.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointN8LAN2.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices4;
    p2pDevices4 = pointToPointN8LAN2.Install(p2pNodesN8LAN2); // duplication des noeuds a ce niveau 

    CsmaHelper csma1; // First CSMA bus
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices1;
    csmaDevices1 = csma1.Install(csmaNodes1);

    CsmaHelper csma2; // Second CSMA bus
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices2;
    csmaDevices2 = csma2.Install(csmaNodes2);

    CsmaHelper csma3; // Second CSMA bus
    csma3.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices3;
    csmaDevices3 = csma3.Install(csmaNodes3);
    

    InternetStackHelper stack;
    //stack.Install(p2pNodes.Get(0)); // Install stack on the first node of the point-to-point link
    stack.Install(csmaNodes1);
    stack.Install(csmaNodes2);
    stack.Install(csmaNodes3);
    stack.Install(p2pNodesLAN3N8.Get(1)); //Installation de la stack sur le noeud N8

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0"); // Tous les équipements avec des conenxions p2p sont dans le même LAN
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);
    p2pInterfaces = address.Assign(p2pDevices1);
    p2pInterfaces = address.Assign(p2pDevices2);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces1;
    csmaInterfaces1 = address.Assign(csmaDevices1);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces2;
    csmaInterfaces2 = address.Assign(csmaDevices2);

    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces3;
    csmaInterfaces3 = address.Assign(csmaDevices3);

    /*// Créer une application BulkSend et l'installer sur le premier nœud
    uint16_t port = 9;  // numéro de port bien connu pour l'écho
    BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress(csmaInterfaces2.GetAddress(1), port));
    ApplicationContainer sourceApps = source.Install(csmaNodes2.Get(1));
    sourceApps.Start(Seconds (1.0));
    sourceApps.Stop(Seconds (20.0));

    // Créer une application PacketSink et l'installer sur le deuxième nœud
    PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(csmaNodes3.Get(1));
    sinkApps.Start (Seconds (1.0));
    sinkApps.Stop (Seconds (20.0));*/

    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
    ApplicationContainer serverApps = packetSinkHelper.Install(csmaNodes3.Get(1)); // Install on the server
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(30.0));

    BulkSendHelper bulkSend("ns3::TcpSocketFactory", InetSocketAddress(csmaInterfaces3.GetAddress(1), 9));
    bulkSend.SetAttribute("MaxBytes", UintegerValue(60)); // 0 for no limit

    // Install the TCP client on a node
    ApplicationContainer clientApps = bulkSend.Install(csmaNodes1.Get(1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(30.0));


    /*UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(csmaNodes3.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));*/

    // Creation d'un 2eme Serveur : 

    UdpEchoServerHelper echoServer2(9);
    ApplicationContainer serverApps2 = echoServer2.Install(csmaNodes1.Get(0));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(30.0));

    /*UdpEchoClientHelper echoClient(csmaInterfaces3.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(packets));
    echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(100)));  // Envoyer un paquet tous les 100 millisecondes
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));*/

    UdpEchoClientHelper echoClient2(csmaInterfaces1.GetAddress(0), 9);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(60));
    echoClient2.SetAttribute("Interval", TimeValue(MilliSeconds(100)));  // Envoyer un paquet tous les 100 millisecondes
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    //ApplicationContainer clientApps = echoClient.Install(csmaNodes1.Get(1)); //verification si csmaNodes2.Get(0) == p2pNodes.Get(0) et csmaNodes1.Get(0) == p2pNodes.Get(1)
    
    //ApplicationContainer clientApps1 = echoClient2.Install(p2pNodesLAN3N8.Get(1));
    ApplicationContainer clientApps1 = echoClient2.Install(csmaNodes2.Get(1)); //verification si csmaNodes2.Get(0) == p2pNodes.Get(0) et csmaNodes1.Get(0) == p2pNodes.Get(1)

    /*clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(stop));*/

    // Démarrage du 2eme client : 
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(30));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); // Mise à jour des tables de routage après la panne du noeud

    //pointToPoint.EnablePcapAll("PCAP/p2pR1R2");
    csma3.EnablePcap("PCAP/server", csmaDevices3.Get(1), true); // Enable PCAP for server
    csma1.EnablePcap("PCAP/client", csmaDevices1.Get(1), true); // Enable PCAP for client


    // Enable Ascii trace output    
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("TRACE/1.tr"));

    // Flow monitor netanim feature activation 

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Trace routing tables 
    Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("project.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (12), routingStream);

    // Net anim configuration 
    AnimationInterface anim("NETANIM/topology.xml");
    
    anim.SetConstantPosition(p2pNodes.Get(0), 24.0, 38.0);
    anim.UpdateNodeColor (p2pNodes.Get(0), 0, 0, 0);
    
    anim.SetConstantPosition(p2pNodes.Get(1), 48.0, 21.5);
    anim.UpdateNodeColor (p2pNodes.Get(1), 0, 255, 0);
    
    anim.SetConstantPosition(p2pNodesLAN2toLAN3.Get(1), 48.0, 57.0);
    anim.UpdateNodeColor (p2pNodesLAN2toLAN3.Get(1), 255, 0, 0);

    anim.SetConstantPosition(p2pNodesLAN3N8.Get(1), 72.0, 38.0);// Positionnement du noeud N8 : 
    anim.UpdateNodeColor(p2pNodesLAN3N8.Get(1), 255, 255, 0);
    
    anim.SetConstantPosition(csmaNodes1.Get(1), 0.0, 21.5);
    anim.UpdateNodeColor (csmaNodes1.Get(1), 0, 0, 0);
    anim.SetConstantPosition(csmaNodes1.Get(2), 0.0, 64.5);
    anim.UpdateNodeColor (csmaNodes1.Get(2), 0, 0, 0);

    anim.SetConstantPosition(csmaNodes2.Get(1), 24.0, 0.0);
    anim.UpdateNodeColor (csmaNodes2.Get(1), 0, 255, 0);
    anim.SetConstantPosition(csmaNodes2.Get(2), 72.0, 0.0);
    anim.UpdateNodeColor (csmaNodes2.Get(2), 0, 255, 0);

    anim.SetConstantPosition(csmaNodes3.Get(1), 24.0, 86.0);
    anim.UpdateNodeColor (csmaNodes3.Get(1), 255, 0, 0);
    anim.SetConstantPosition(csmaNodes3.Get(2), 72.0, 86.0);
    anim.UpdateNodeColor (csmaNodes3.Get(2), 255, 0, 0);

    Simulator::Stop(Seconds(60));
    Simulator::Run();  

    // Classification des flux : 

    std::cout << "################## RUN ###############\n" << std::endl ;
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end(); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
            std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << " Tx Bytes: " << i->second. txBytes << "\n";
            std::cout << " RX bytes: " << i->second.rxBytes << "\n";
            std::cout << " tx Packets: " << i->second.txPackets << "\n";
            std::cout << " Rx Packets: " << i->second.rxPackets << "\n";
           // std::cout << " Lost Packets: " << i->second.lostPackets << "\n";
            //std::cout << " Times Forwarded: " << i->second.timesForwarded << "\n";
           // std::cout << " Delay Sum: " << i->second.delaySum.GetSeconds() << "\n";
            std::cout << " Throughput: " << i->second.rxBytes* 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024 << "kbps\n";
    }    

    Simulator::Destroy();

    flowMonitor->SerializeToXmlFile("NETANIM/flowMonitor",true,true);
    return 0;
}
