#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);

  // Create nodes
  NodeContainer nodes;
  nodes.Create (2);

  // Create link
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devices = p2p.Install (nodes);

  // Install internet stack
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IP
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  uint16_t port = 8080;

  // ---------------- SERVER ----------------
  PacketSinkHelper server ("ns3::TcpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));

  ApplicationContainer serverApp = server.Install (nodes.Get (1));
  serverApp.Start (Seconds (1.0));
  serverApp.Stop (Seconds (15.0));

  // ---------------- CLIENT ----------------
  OnOffHelper client ("ns3::TcpSocketFactory",
                      InetSocketAddress (interfaces.GetAddress (1), port));

  client.SetAttribute ("DataRate", StringValue ("1Mbps"));
  client.SetAttribute ("PacketSize", UintegerValue (1024));
  client.SetAttribute ("OnTime",
    StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  client.SetAttribute ("OffTime",
    StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer clientApp = client.Install (nodes.Get (0));
  clientApp.Start (Seconds (2.0));
  clientApp.Stop (Seconds (10.0));

  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();

  Ptr<PacketSink> sink = DynamicCast<PacketSink> (serverApp.Get (0));

  std::cout << "Total Bytes Received: "
            << sink->GetTotalRx ()
            << std::endl;

  Simulator::Destroy ();
  return 0;
}