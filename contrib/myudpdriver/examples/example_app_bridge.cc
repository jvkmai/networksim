#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/bridge-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nstime.h"
#include "ns3/tsn-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/config-store-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/tsn-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/config-store-module.h"

#include "../helper/publisher_helper.h"
#include "../helper/subscriber_helper.h"
#include "../helper/RTPS-UdpDriverHelper.h"
#include "../model/app-publisher.h"
#include "../model/app-subscriber.h"
#include "../model/rtps-udpdriver.h"

#include "stdio.h"
#include <array>
#include <string>
#include <chrono>

#define NUMBER_OF_SCHEDULE_ENTRYS 20000

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyRtpsExample");

Time callbackfunc();
int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item);

int main (int argc, char *argv[]) {

  rtps::init();
  Time::SetResolution (Time::NS);

  // Enable logs
  //LogComponentEnable ("MyRtpsExample", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpDriver", LOG_LEVEL_INFO);
  //LogComponentEnable ("MyRtpsPublisherApp", LOG_LEVEL_INFO);
  //LogComponentEnable ("MyRtpsSubscriberApp", LOG_LEVEL_INFO);
  NS_LOG_INFO ("Starting program");

  // Create four nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (4);

  NS_LOG_INFO ("Create bridge node.");
  NodeContainer bridgeNode;
  bridgeNode.Create (1);

  // Connect nodes with a csma link.
  NS_LOG_INFO ("Create csma channel.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("0ms"));

  NS_LOG_INFO ("Create first csma channel.");
  NodeContainer csmaNodes1;
  csmaNodes1.Add(nodes.Get(0)); // Add node0
  csmaNodes1.Add(bridgeNode.Get(0)); // Add bridge node
  NetDeviceContainer csmaDevices1 = csma.Install(csmaNodes1); // Install the CSMA channel

  NS_LOG_INFO ("Create second csma channel.");
  NodeContainer csmaNodes2;
  csmaNodes2.Add(nodes.Get(1)); // Add node1
  csmaNodes2.Add(nodes.Get(2)); // Add node2
  csmaNodes2.Add(nodes.Get(3)); // Add node3
  csmaNodes2.Add(bridgeNode.Get(0)); // Add bridge node
  NetDeviceContainer csmaDevices2 = csma.Install(csmaNodes2); // Install the CSMA channel

  // Install the internet stack on the nodes
  NS_LOG_INFO ("Install internet stack on nodes.");
  InternetStackHelper stack;
  stack.Install (nodes);
  stack.Install (bridgeNode);

  NS_LOG_INFO ("Configure the bridge node.");
  BridgeHelper bridge;
  NetDeviceContainer bridgeDevices;
  bridgeDevices.Add (csmaDevices1.Get (1));
  bridgeDevices.Add (csmaDevices2.Get (3));
  bridge.Install (bridgeNode.Get (0), bridgeDevices);

  // Setup TSN on all nodes
  CallbackValue timeSource = MakeCallback(&callbackfunc);
  TsnHelper tsnHelperNode0, tsnHelperNode1, tsnHelperNode2, tsnHelperNode3;
  NetDeviceListConfig schedulePlanNode0, schedulePlanNode1, schedulePlanNode2, schedulePlanNode3;

  for (int i = 0; i < NUMBER_OF_SCHEDULE_ENTRYS; i++) {
    //schedulePlanNode0.Add(MilliSeconds(1),{0,0,0,0,1,0,0,0});
    //schedulePlanNode0.Add(MilliSeconds(1),{0,1,0,0,0,0,0,0});
    schedulePlanNode0.Add(MilliSeconds(3),{1,0,1,1,0,1,1,1});
    //schedulePlanNode0.Add(MilliSeconds(6),{1,1,1,1,1,1,1,1});
  }

  tsnHelperNode0.SetRootQueueDisc("ns3::TasQueueDisc", "NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanNode0), "TimeSource", timeSource,"DataRate", StringValue ("100Mbps"));
  tsnHelperNode0.AddPacketFilter(0,"ns3::TsnIpv4PacketFilter","Classify",CallbackValue(MakeCallback(&ipv4PacketFilter)));
  
  QueueDiscContainer qdiscsNode0 = tsnHelperNode0.Install (csmaDevices1.Get(1));
  QueueDiscContainer qdiscsNode1 = tsnHelperNode0.Install (csmaDevices2.Get(3));

  // Assign IP addresses.
  NS_LOG_INFO ("Assign IP addresses.");
  Ipv4AddressHelper address;
  address.SetBase ("137.226.8.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (csmaDevices1);
  Ipv4InterfaceContainer interfaces2 = address.Assign (csmaDevices2);

  // // Change IP address of node 0 to "137.226.8.70"
  // Ptr<Node> node0 = nodes.Get (0);
  // Ptr<Ipv4> ipv4 = node0->GetObject<Ipv4> ();
  // int32_t interface =  ipv4->GetInterfaceForAddress (interfaces1.GetAddress (0, 0));
  // ipv4->RemoveAddress(interface, 0);
  // ipv4->AddAddress(interface, Ipv4InterfaceAddress (Ipv4Address ("137.226.8.70"), Ipv4Mask ("255.255.255.0")));

  // // Change IP address of node 1 to "137.226.8.71"
  // Ptr<Node> node1 = nodes.Get (1);
  // ipv4 = node1->GetObject<Ipv4> ();
  // interface =  ipv4->GetInterfaceForAddress (interfaces2.GetAddress (0, 0));
  // ipv4->RemoveAddress(interface, 0);
  // ipv4->AddAddress(interface, Ipv4InterfaceAddress (Ipv4Address ("137.226.8.73"), Ipv4Mask ("255.255.255.0")));

  // // Change IP address of node 2 to "137.226.8.72"
  // Ptr<Node> node2 = nodes.Get (2);
  // ipv4 = node2->GetObject<Ipv4> ();
  // interface =  ipv4->GetInterfaceForAddress (interfaces2.GetAddress (1, 0));
  // ipv4->RemoveAddress(interface, 0);
  // ipv4->AddAddress(interface, Ipv4InterfaceAddress (Ipv4Address ("137.226.8.74"), Ipv4Mask ("255.255.255.0")));

  // // Change IP address of node 3 to "137.226.8.73"
  // Ptr<Node> node3 = nodes.Get (3);
  // ipv4 = node3->GetObject<Ipv4> ();
  // interface =  ipv4->GetInterfaceForAddress (interfaces2.GetAddress (2, 0));
  // ipv4->RemoveAddress(interface, 0);
  // ipv4->AddAddress(interface, Ipv4InterfaceAddress (Ipv4Address ("137.226.8.75"), Ipv4Mask ("255.255.255.0")));
  
  
  // Setup multicast routing
  // Every node will get a multicast route to every other node
  Ipv4Address multicastGroup("239.255.0.1");
  Ipv4StaticRoutingHelper multicast;

  for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    Ptr<Node> sender = nodes.Get(i);
    Ptr<NetDevice> senderIf;
    Ipv4Address senderAddress;
    if (i == 0) {
      senderIf = csmaDevices1.Get(0);
      senderAddress = interfaces1.GetAddress(0, 0);
    } else {
      senderIf = csmaDevices2.Get(i-1);
      senderAddress = interfaces2.GetAddress(i-1, 0);
    }
  
    for (uint32_t j = 0; j < nodes.GetN(); ++j) {
      if (j != i) {
        Ptr<Node> multicastRouter = nodes.Get(j);
        Ptr<NetDevice> inputIf;
        Ipv4Address multicastSource;
      
        if (j == 0) {
          inputIf = csmaDevices1.Get(0);
          multicastSource = interfaces1.GetAddress(0, 0);
        } else {
          inputIf = csmaDevices2.Get(j-1);
          multicastSource = interfaces2.GetAddress(j-1, 0);
        }
      
        multicast.AddMulticastRoute(multicastRouter, senderAddress, multicastGroup, inputIf, NetDeviceContainer());
        multicast.SetDefaultMulticastRoute(sender, senderIf);
      
        Ipv4Address reverseMulticastSource = multicastSource;
        Ptr<Node> reverseMulticastRouter = sender;
        Ptr<NetDevice> reverseInputIf = senderIf;
      
        multicast.AddMulticastRoute(reverseMulticastRouter, reverseMulticastSource, multicastGroup, reverseInputIf, NetDeviceContainer());
        multicast.SetDefaultMulticastRoute(multicastRouter, inputIf);
        
      }
    }
  }
  

  // Publisher application on first node.
  NS_LOG_INFO ("Create publisher app.");
  PublisherAppHelper publisherHelper(interfaces1.GetAddress(0));
  ApplicationContainer publisherApp = publisherHelper.Install(nodes.Get(0));
  publisherApp.Start(Seconds(1.0));
  publisherApp.Stop(Seconds(10.0));

  // Subscriber application on second node.
  NS_LOG_INFO ("Create subscriber app.");
  SubscriberAppHelper subscriberHelper1(interfaces2.GetAddress(0));
  subscriberHelper1.SetAttribute("TopicName", StringValue("HELLOEARTH"));
  subscriberHelper1.SetAttribute("TopicType", StringValue("TEST"));
  ApplicationContainer subscriberApp1 = subscriberHelper1.Install(nodes.Get(1));
  subscriberApp1.Start(Seconds(0.0));
  subscriberApp1.Stop(Seconds(20.0));

  // Subscriber application on third node.
  NS_LOG_INFO ("Create subscriber app.");
  SubscriberAppHelper subscriberHelper2(interfaces2.GetAddress(1));
  subscriberHelper2.SetAttribute("TopicName", StringValue("HELLOMARS"));
  subscriberHelper2.SetAttribute("TopicType", StringValue("TEST"));
  ApplicationContainer subscriberApp2 = subscriberHelper2.Install(nodes.Get(2));
  subscriberApp2.Start(Seconds(0.0));
  subscriberApp2.Stop(Seconds(20.0));

  // Subscriber application on fourth node.
  NS_LOG_INFO ("Create subscriber app.");
  SubscriberAppHelper subscriberHelper3(interfaces2.GetAddress(2));
  subscriberHelper3.SetAttribute("TopicName", StringValue("HELLOSATURN"));
  subscriberHelper3.SetAttribute("TopicType", StringValue("TEST"));
  ApplicationContainer subscriberApp3 = subscriberHelper3.Install(nodes.Get(3));
  subscriberApp3.Start(Seconds(0.0));
  subscriberApp3.Stop(Seconds(20.0));

  FlowMonitorHelper monitor;
  Ptr<FlowMonitor> flowmonitor = monitor.InstallAll();

  csma.EnablePcapAll ("myexample-rtps");

  Simulator::Stop (Seconds (25.0)); 
  Simulator::Run ();

  flowmonitor->SerializeToXmlFile ("myexample-rtps-flowmon.xml", false, false);

  Simulator::Destroy ();

  return 0;
}

// Set this filter to sort the packets into the correct queues
// Return value is the queue number (0-7)
int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item){
  Ptr<Packet> packet = item->GetPacket()->Copy();
  uint32_t packetSize = packet->GetSize();
  uint8_t* buffer = new uint8_t[packetSize];
  packet->CopyData(buffer, packetSize);
  std::string packetString(reinterpret_cast<char*>(buffer), packetSize);
  delete[] buffer;

  if (packetString.find("Earth") != std::string::npos) {
    NS_LOG_INFO("Queued Earth packet");
    return 1;
  }
  if (packetString.find("Saturn") != std::string::npos) {
    NS_LOG_INFO("Queued Saturn packet");
    return 1;
  }
  return 1;
}

Time callbackfunc(){
  return Simulator::Now();
}