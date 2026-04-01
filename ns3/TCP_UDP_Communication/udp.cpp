#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
  {
    std::cout << "UDP Server received "
              << packet->GetSize ()
              << " bytes at "
              << Simulator::Now ().GetSeconds ()
              << " s\n";
  }
}

void SendPacket (Ptr<Socket> socket, uint32_t size, uint32_t count, Time interval)
{
  if (count > 0)
  {
    socket->Send (Create<Packet> (size));

    Simulator::Schedule (interval, &SendPacket, socket, size, count - 1, interval);
  }
  else
  {
    socket->Close ();
  }
}

int main ()
{
  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices = p2p.Install (nodes);

  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  uint16_t port = 8080;

  // SERVER (UDP)
  Ptr<Socket> server =
    Socket::CreateSocket (nodes.Get (1), UdpSocketFactory::GetTypeId ());

  server->Bind (InetSocketAddress (Ipv4Address::GetAny (), port));
  server->SetRecvCallback (MakeCallback (&ReceivePacket));

  // CLIENT (UDP)
  Ptr<Socket> client =
    Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());

  client->Connect (InetSocketAddress (interfaces.GetAddress (1), port));

  Simulator::Schedule (Seconds (2.0), &SendPacket, client, 1024, 10, Seconds (1.0));

  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();
  Simulator::Destroy ();
}