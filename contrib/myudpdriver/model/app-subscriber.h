#ifndef APP_SUBSCRIBER_H
#define APP_SUBSCRIBER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"

#include "rtps/rtps.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Domain.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/Reader.h"
#include "rtps-udpdriver.h"

#include <array>
#include <vector>

using namespace ns3;

class MyRtpsSubscriberApp : public Application {
public:
    MyRtpsSubscriberApp();
    virtual ~MyRtpsSubscriberApp();
    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const override;
    
protected:
    virtual void StartApplication();
    virtual void StopApplication();

private:
    static void ReceiveCallback(void* callee, const rtps::ReaderCacheChange& cacheChange);
    void WriteTimingDataToTextFile();

    struct TimeData {
    int64_t sendTime;
    int64_t receiveTime;
    int64_t timeDiff;
    int msgId;
    int64_t udpSendTime;
};

    Address m_destAddr;
    Address m_localAddress;
    std::array<uint8_t, 4> IP_ADDRESS;
    std::string m_topicName;
    std::string m_topicType;

    ObjectFactory m_factory;
    std::unique_ptr<rtps::Domain> m_domain;
    rtps::Participant* m_participant;
    rtps::Reader* m_reader;
    Time m_interval;
    std::string m_message;
    std::vector<TimeData> m_timeData;

};

#endif /* MY_RTPS_SUBSCRIBER_APP_H */
