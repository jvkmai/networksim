#include "rtps-udpdriver.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <thread>

#include "rtps/communication/TcpipCoreLock.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

#include "lwip/udp.h"
#include <lwip/igmp.h>
#include <lwip/tcpip.h>

#if UDP_DRIVER_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define UDP_DRIVER_LOG(...)                                                    \
  if (true) {                                                                  \
    printf("[UDP Driver] ");                                                   \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define UDP_DRIVER_LOG(...) //
#endif

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpDriver");

NS_OBJECT_ENSURE_REGISTERED (UdpDriver);

TypeId
UdpDriver::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::UdpDriver")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<UdpDriver> ()
    .AddAttribute ("RemoteAddress", "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpDriver::m_destAddr),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpDriver::m_destPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxPackets", "The maximum number of packets the application will send",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&UdpDriver::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", "The time to wait between packets",
                   TimeValue (MicroSeconds (1.0)),
                   MakeTimeAccessor (&UdpDriver::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("PacketSize", "Size of packets generated",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&UdpDriver::m_size),
                   MakeUintegerChecker<uint32_t> (12,65507))
    .AddAttribute ("VLANset", "The VLAN flag to be set in payload",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpDriver::m_vlan),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LocalPort", "The port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpDriver::m_localPort),
                   MakeUintegerChecker<uint16_t> ())
    ;
  return tid;
}

TypeId UdpDriver::GetInstanceTypeId() const
{
  return UdpDriver::GetTypeId();
}

UdpDriver::UdpDriver(udpRxFunc_fp callback, void *args) 
    : m_rxCallback(callback), m_callbackArgs(args)
{
    NS_LOG_FUNCTION (this);
    m_socket = nullptr;
    m_socketIsBound = false;
    m_sent = 0;
    m_sendEvent = EventId();
} 

UdpDriver::~UdpDriver()
{ 
    NS_LOG_FUNCTION (this);
    m_socket = nullptr;
}

void UdpDriver::setRcvCallback(udpRxFunc_fp callback, void *args) {
    m_rxCallback = callback;
    m_callbackArgs = args;
}

void
UdpDriver::createUdpConnection(rtps::Ip4Port_t receivePort) {
  NS_LOG_INFO("[UdpDriver] Creating UDP connection on node " << GetNode() << ". Port: " << receivePort); 
  m_socket = DynamicCast<UdpSocket>(Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ()));
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), receivePort);
  if (m_socket->Bind(local) == -1) {
    NS_LOG_INFO ("Failed to bind socket");
  }
  m_socketIsBound = true;
  m_socket->SetRecvCallback(MakeCallback(&UdpDriver::HandleRead, this));
  m_socket->SetAllowBroadcast(true);
}

bool UdpDriver::isSameSubnet(ip4_addr_t addr) {
  // return (ip4_addr_netcmp(&addr, &(netif_default->ip_addr),
  //                         &(netif_default->netmask)) != 0);
  return true;
}

bool UdpDriver::isMulticastAddress(ip4_addr_t addr) {
  uint32_t destAddrHostByteOrder = ntohl(addr.addr);
  std::string destAddrString = std::to_string((destAddrHostByteOrder >> 24) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder >> 16) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder >>  8) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder      ) & 0xFF);
  Ipv4Address m_multicast_addr(destAddrString.c_str());
  return m_multicast_addr.IsMulticast();
}


bool UdpDriver::joinMultiCastGroup(ip4_addr_t addr) const {
  uint32_t destAddrHostByteOrder = ntohl(addr.addr);
  std::string destAddrString = std::to_string((destAddrHostByteOrder >> 24) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder >> 16) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder >>  8) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder      ) & 0xFF);
  Ipv4Address local_multicastAddr(destAddrString.c_str());
  Address groupAddress = ns3::InetSocketAddress(local_multicastAddr, 0); 
  return true;
  uint32_t interface = 0;
  // if (m_socket->MulticastJoinGroup(interface, groupAddress) == 0) {
  //   NS_LOG_INFO ("Successfully joined multicast group " << local_multicastAddr);
  //   std::cout << "Successfully joined multicast group " << local_multicastAddr << std::endl;
  //   return true;
  //   } 
  // else {
  //   NS_LOG_INFO ("Failed to join multicast group " << groupAddress);
  //   return false;
  // }
}

void UdpDriver::sendPacket(ip4_addr_t &destAddr, rtps::Ip4Port_t destPort, pbuf *buffer)
{
  uint32_t destAddrHostByteOrder = ntohl(destAddr.addr);
  std::string destAddrString = std::to_string((destAddrHostByteOrder >> 24) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder >> 16) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder >>  8) & 0xFF) + "." +
                               std::to_string((destAddrHostByteOrder      ) & 0xFF);
  Ipv4Address local_destAddr(destAddrString.c_str());
  local_destPort = uint16_t(destPort);
  uint8_t* payload = static_cast<uint8_t*>(buffer->payload); 

  // std::string payloadStr((char*)payload, buffer->len);
  // std::string searchString = "Creation Time: ";
  // size_t found = payloadStr.find(searchString);
  size_t newPayloadLen = buffer->len;

  // if (found != std::string::npos) {
  //     found += searchString.size();
  //     size_t endPos = payloadStr.find(' ', found);
  //     if(endPos == std::string::npos) {
  //       endPos = payloadStr.length();
  //     }
  //     std::string sendTime = " UDP SendTime: " + std::to_string(Simulator::Now().GetMicroSeconds()+ '\0');
  //     newPayloadLen += sendTime.size();
  // }
  uint8_t* newPayload = new uint8_t[newPayloadLen];
  std::memcpy(newPayload, payload, buffer->len);

  // if (found != std::string::npos) {
  //   std::string sendTime = " UDP SendTime: " + std::to_string(Simulator::Now().GetMicroSeconds()+ '\0');
  //   std::memcpy(newPayload + buffer->len, sendTime.data(), sendTime.size());
  // }

  Ptr<Packet> packet = Create<Packet>(newPayload, newPayloadLen);
  delete[] newPayload;

  if (packet == 0) {
    NS_LOG_ERROR ("[UdpDriver::sendPacket] Failed to create packet");
  }
  if (!m_socketIsBound) { 
    NS_LOG_INFO ("[UdpDriver::sendPacket] Binding socket");
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), local_destPort);
    if (m_socket->Bind(local) == -1) {
      NS_LOG_ERROR ("[UdpDriver::sendPacket] Failed to bind socket");
    }
    m_socketIsBound = true;
  }
  if (Ipv4Address::IsMatchingType(local_destAddr) == true) {    
    if(m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(local_destAddr), local_destPort)) == -1) {
      NS_LOG_ERROR ("[UdpDriver::sendPacket] Failed to connect socket");
    }
    NS_LOG_INFO ("[UdpDriver::sendPacket] Socket connected to " << local_destAddr << ":" << local_destPort);
  }
  else if (InetSocketAddress::IsMatchingType(local_destAddr) == true) {
    if(m_socket->Connect(local_destAddr) == -1) {
      NS_LOG_ERROR ("[UdpDriver::sendPacket] Failed to connect socket");
    }
    NS_LOG_INFO ("[UdpDriver::sendPacket] Socket connected to " << local_destAddr);
  }
  NS_LOG_INFO ("[UdpDriver::sendPacket] Sending packet to " << local_destAddr << ":" << local_destPort);
  NS_LOG_INFO ("[UdpDriver::sendPacket] Packet size: " << packet->GetSize());
  int ret = m_socket->Send(packet);
  pbuf_free(buffer);
  if (ret == -1) {
    NS_LOG_ERROR ("Send failed with error: " << m_socket->GetErrno());
  }
  else {
  }
}

void 
UdpDriver::sendPacket(rtps::PacketInfo &packet) {
  if(packet.buffer.firstElement != nullptr) {
    auto chainedBuffer = pbuf_coalesce(packet.buffer.firstElement, PBUF_TRANSPORT);
    if (chainedBuffer == packet.buffer.firstElement) {
      NS_LOG_ERROR ("[UdpDriver::sendPacket] pbuf_coalesce failed to create a single contiguous buffer");
    }
    sendPacket(packet.destAddr, packet.destPort, chainedBuffer);
  }
  else {
    NS_LOG_ERROR ("[UdpDriver::sendPacket] packet.buffer.firstElement is nullptr");
  }
}

void 
UdpDriver::StartApplication()
{
  NS_LOG_FUNCTION(this);
    if (!m_socket) {    
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    Ptr<Node> node = GetNode();
    assert(node != nullptr);
    NS_LOG_INFO("[UdpDriver::StartApplication]  Node: " << node);
    m_socket = DynamicCast<UdpSocket>(Socket::CreateSocket (node, tid));
    assert(m_socket != nullptr);
    NS_LOG_INFO("[UdpDriver::StartApplication] Socket: " << m_socket);

    m_socket->SetRecvCallback(MakeCallback(&UdpDriver::HandleRead, this));
    }
}

void 
UdpDriver::StopApplication() 
{
  NS_LOG_FUNCTION(this);
  if (m_socket)
  {
    m_socket->Close();
    m_socket = nullptr;
  }
}

pbuf* 
UdpDriver::PacketToPbuf(Ptr<Packet> packet)
{
  uint32_t pktSize = packet->GetSize();
  std::vector<uint8_t> buffer(pktSize);
  packet->CopyData(buffer.data(), pktSize);

  pbuf *p = pbuf_alloc(PBUF_TRANSPORT, pktSize, PBUF_RAM);
  if (!p) {
      NS_LOG_ERROR("Failed to allocate pbuf");
      return nullptr;
    }

  if (pbuf_take(p, buffer.data(), pktSize) != ERR_OK) {
      NS_LOG_ERROR("Failed to copy data to pbuf");
      pbuf_free(p);
      return nullptr;
    }
    return p;
}

void 
UdpDriver::HandleRead(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom(from))) {
    if (packet == nullptr) {
      NS_LOG_ERROR("Received null packet");
      continue;
    }

    // uint32_t packetSize = packet->GetSize();
    // uint8_t* buffer = new uint8_t[packetSize];
    // packet->CopyData(buffer, packetSize);
    // uint16_t local_destPort_nbo;
    // std::memcpy(&local_destPort_nbo, buffer + 8, sizeof(local_destPort_nbo));
    // rtps::Ip4Port_t destPort = ntohs(local_destPort_nbo);

    // size_t newPayloadLen = packetSize - sizeof(local_destPort_nbo);
    // uint8_t* newPayload = new uint8_t[newPayloadLen];
    // std::memcpy(newPayload, buffer, 8);
    // std::memcpy(newPayload + 8, buffer + 10, newPayloadLen - 8);
    // Ptr<Packet> newPacket = Create<Packet>(newPayload, newPayloadLen);
    // delete[] buffer;
    // delete[] newPayload; 
    
    uint32_t packetSize = packet->GetSize();
    uint8_t* buffer = new uint8_t[packetSize];
    packet->CopyData(buffer, packetSize);
    
    size_t newPayloadLen = packetSize;
    uint8_t* newPayload = new uint8_t[newPayloadLen];
    std::memcpy(newPayload, buffer, newPayloadLen);
    Ptr<Packet> newPacket = Create<Packet>(newPayload, newPayloadLen);
    delete[] buffer;
    delete[] newPayload; 

    pbuf *p = PacketToPbuf(newPacket);
    if (!p) {
      NS_LOG_ERROR("Failed to convert packet to pbuf");
      continue;
    }
    socket->GetSockName(localAddress);
    uint16_t destPort = InetSocketAddress::ConvertFrom(localAddress).GetPort();
    NS_LOG_INFO("[UdpDriver::HandleReader] At time " << Simulator::Now().GetMicroSeconds() << "us" 
              << " client received "
              << packet->GetSize() << " bytes from "
              << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
              << InetSocketAddress::ConvertFrom(from).GetPort());

    ip_addr_t addr = {0}; // Not used
    rtps::Ip4Port_t port = 9; // Not used
    m_rxCallback(m_callbackArgs, destPort, p, &addr, port);
  }
}

