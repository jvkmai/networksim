#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"

#include "rtps/communication/UdpConnection.h"
#include "lwip/udp.h"
#include "rtps/common/types.h"
#include "rtps/communication/PacketInfo.h"
#include "rtps/config.h"
#include "rtps/storages/PBufWrapper.h"

#include <array>

namespace ns3 {

class UdpDriver : public ns3::Application{

public:
typedef void (*udpRxFunc_fp)(void *arg, rtps::Ip4Port_t destPort, pbuf *p,
                              const ip_addr_t *addr, rtps::Ip4Port_t port);

  UdpDriver(udpRxFunc_fp callback, void *args);
  void createUdpConnection(rtps::Ip4Port_t receivePort);
  static ns3::TypeId GetTypeId();
  ns3::TypeId GetInstanceTypeId() const;
  UdpDriver() 
    : m_rxCallback(nullptr), m_callbackArgs(nullptr) {}
  ~UdpDriver();

  bool joinMultiCastGroup(ip4_addr_t addr) const;
  void sendPacket(rtps::PacketInfo &info);

  static bool isSameSubnet(ip4_addr_t addr);
  static bool isMulticastAddress(ip4_addr_t addr);

  virtual void StartApplication() override;
  virtual void StopApplication() override;

  void SetRemote(ns3::Address ip, uint16_t port);
  void HandleRead(ns3::Ptr<ns3::Socket> socket);
  pbuf* PacketToPbuf(ns3::Ptr<ns3::Packet> packet);

  void setRcvCallback(udpRxFunc_fp callback, void *args);

private:
  uint32_t m_maxpacket;
  Time m_interval;
  uint32_t m_size;
  uint32_t m_vlan;
  uint32_t m_sent;
  uint32_t m_count;
  
  Ptr<UdpSocket> m_socket;
  Ipv4Address local_destAddr;
  Address m_destAddr;
  uint16_t local_destPort;
  rtps::Ip4Port_t m_destPort;
  uint16_t m_localPort;
  EventId m_sendEvent;
  bool m_socketIsBound;

  std::array<rtps::UdpConnection, rtps::Config::MAX_NUM_UDP_CONNECTIONS> m_conns;
  std::size_t m_numConns = 0;
  udpRxFunc_fp m_rxCallback = nullptr;
  void *m_callbackArgs = nullptr;

  void sendPacket(ip4_addr_t &destAddr, rtps::Ip4Port_t destPort, pbuf *buffer);
};
}
#endif
