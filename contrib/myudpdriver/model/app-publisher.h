#ifndef APP_PUBLISHER_H
#define APP_PUBLISHER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"

#include "rtps/rtps.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Domain.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/Writer.h"

#include "rtps-udpdriver.h"

#include <vector>

using namespace ns3;

class MyRtpsPublisherApp : public Application {
public:
    MyRtpsPublisherApp();
    virtual ~MyRtpsPublisherApp();
    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const override;

protected:
    virtual void StartApplication();
    virtual void StopApplication();

private:
    void SendMessageEarth();
    void SendMessageMars();
    void SendMessageSaturn();

    uint16_t m_destPort;
    Address m_localAddress;
    Time m_interval;
    Time m_creationTime;
    uint32_t m_msgIdEarth;
    uint32_t m_msgIdMars;
    uint32_t m_msgIdSaturn;

    std::array<uint8_t, 4> IP_ADDRESS;
    ObjectFactory m_factory;
    std::unique_ptr<rtps::Domain> m_domain;
    rtps::Participant* m_participant;
    rtps::Writer* m_writer1;
    rtps::Writer* m_writer2;
    rtps::Writer* m_writer3;
    EventId m_sendEventEarth;
    EventId m_sendEventMars;
    EventId m_sendEventSaturn;
    std::string m_message;
};

#endif /* MY_RTPS_PUBLISHER_APP_H */
