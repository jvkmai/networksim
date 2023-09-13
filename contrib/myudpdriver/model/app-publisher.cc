#include "app-publisher.h"

#include "rtps/communication/UdpDriver.h"
#include "rtps/communication/TcpipCoreLock.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/Writer.h"

#include <lwip/igmp.h>
#include <lwip/tcpip.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyRtpsPublisherApp");

NS_OBJECT_ENSURE_REGISTERED (MyRtpsPublisherApp);

TypeId
MyRtpsPublisherApp::GetTypeId(void)
{
  static TypeId tid = TypeId ("ns3::MyRtpsPublisherApp")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<MyRtpsPublisherApp>()
    .AddAttribute ("LocalAddress", "The local Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&MyRtpsPublisherApp::m_localAddress),
                   MakeAddressChecker ())              
    .AddAttribute ("Interval", "The time to wait between sending messages",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&MyRtpsPublisherApp::m_interval),
                   MakeTimeChecker());
  return tid;
}

TypeId
MyRtpsPublisherApp::GetInstanceTypeId(void) const
{
  return MyRtpsPublisherApp::GetTypeId();
}


MyRtpsPublisherApp::MyRtpsPublisherApp() {
    NS_LOG_FUNCTION(this);
    m_msgIdEarth = 0;
    m_msgIdMars = 0;
    m_msgIdSaturn = 0;
}

MyRtpsPublisherApp::~MyRtpsPublisherApp() {
    NS_LOG_FUNCTION(this);
}

void MyRtpsPublisherApp::StartApplication() {
    NS_LOG_FUNCTION(this);

    uint32_t ipv4Addr = ns3::Ipv4Address::ConvertFrom(m_localAddress).Get();
    std::array<uint8_t, 4> ipBytes;
    ipBytes[0] = static_cast<uint8_t>((ipv4Addr >> 24) & 0xFF);
    ipBytes[1] = static_cast<uint8_t>((ipv4Addr >> 16) & 0xFF);
    ipBytes[2] = static_cast<uint8_t>((ipv4Addr >> 8) & 0xFF);
    ipBytes[3] = static_cast<uint8_t>(ipv4Addr & 0xFF);
    IP_ADDRESS = ipBytes;

    Ptr<Node> node = GetNode();
    NS_LOG_INFO("MyRtpsPublisherApp::StartApplication creates UdpDriver on node: " << node << std::endl);
    m_factory.SetTypeId ("ns3::UdpDriver");
    Ptr<UdpDriver> driver = m_factory.Create<UdpDriver>();
    node->AddApplication(driver);
    m_domain = std::make_unique<rtps::Domain>(*driver, IP_ADDRESS);
    m_participant = m_domain->createParticipant(node->GetId());
    m_domain->createBuiltinWritersAndReaders(*m_participant, IP_ADDRESS);
    if(m_participant == nullptr){
        NS_FATAL_ERROR("Failed to create participant");
    }
    m_writer1 = m_domain->createWriter(*m_participant, "HELLOEARTH", "TEST", false);
    NS_LOG_INFO("MyRtpsPublisherApp::StartApplication creates HELLOEARTH writer");
    if (m_writer1 == nullptr) {
        NS_FATAL_ERROR("Failed to create writer");
    }
    m_writer2 = m_domain->createWriter(*m_participant, "HELLOMARS", "TEST", false);
    NS_LOG_INFO("MyRtpsPublisherApp::StartApplication creates HELLOMARS writer");
    if (m_writer2 == nullptr) {
        NS_FATAL_ERROR("Failed to create writer");
    }
    m_writer3 = m_domain->createWriter(*m_participant, "HELLOSATURN", "TEST", false);
    NS_LOG_INFO("MyRtpsPublisherApp::StartApplication creates HELLOSATURN writer");
    if (m_writer3 == nullptr) {
        NS_FATAL_ERROR("Failed to create writer");
    }

    m_domain->completeInit();
    m_participant->getSPDPAgent().start();

    m_sendEventEarth = Simulator::Schedule(Seconds(4.0), &MyRtpsPublisherApp::SendMessageEarth, this);
    m_sendEventMars = Simulator::Schedule(Seconds(5.0), &MyRtpsPublisherApp::SendMessageMars, this);
    m_sendEventSaturn = Simulator::Schedule(Seconds(4.0), &MyRtpsPublisherApp::SendMessageSaturn, this);
}

void MyRtpsPublisherApp::StopApplication() {
    NS_LOG_FUNCTION(this);
    m_domain->stop();
    Simulator::Cancel(m_sendEventEarth);
    Simulator::Cancel(m_sendEventMars);
    Simulator::Cancel(m_sendEventSaturn);
}

void MyRtpsPublisherApp::SendMessageEarth() {
    NS_LOG_FUNCTION(this);
    std::string message_earth = "Hello Earth! ";
    message_earth += "Message ID: " + std::to_string(m_msgIdEarth++);
    message_earth += " Creation Time: " + std::to_string(Simulator::Now().GetMicroSeconds()) + '\0';
    std::vector<uint8_t> buffer1(message_earth.begin(), message_earth.end());
    NS_LOG_INFO("MyRtpsPublisherApp::SendMessageEarth sends message: " << message_earth);
    auto change_earth = m_writer1->newChange(rtps::ChangeKind_t::ALIVE, buffer1.data(), buffer1.size());
    if (change_earth == nullptr) {
        NS_LOG_ERROR("History full.");
        return;
    }
    m_sendEventEarth = Simulator::Schedule(MicroSeconds(10000.0), &MyRtpsPublisherApp::SendMessageEarth, this);
}

void MyRtpsPublisherApp::SendMessageMars() {
    NS_LOG_FUNCTION(this);
    std::string message_mars = "Hello Mars! ";
    message_mars += "Message ID: " + std::to_string(m_msgIdMars++);
    message_mars += " Creation Time: " + std::to_string(Simulator::Now().GetMicroSeconds()) + '\0';
    std::vector<uint8_t> buffer2(message_mars.begin(), message_mars.end());
    NS_LOG_INFO("MyRtpsPublisherApp::SendMessageMars sends message: " << message_mars);
    auto change_mars = m_writer2->newChange(rtps::ChangeKind_t::ALIVE, buffer2.data(), buffer2.size());
    if (change_mars == nullptr) {
        NS_LOG_ERROR("History full.");
        return;
    }
    m_sendEventMars = Simulator::Schedule(MicroSeconds(10.0), &MyRtpsPublisherApp::SendMessageMars, this);
}

void MyRtpsPublisherApp::SendMessageSaturn() {
    NS_LOG_FUNCTION(this);
    std::string message_saturn = "Hello Saturn! ";
    message_saturn += "Message ID: " + std::to_string(m_msgIdSaturn++);
    message_saturn += " Creation Time: " + std::to_string(Simulator::Now().GetMicroSeconds()) + '\0';
    std::vector<uint8_t> buffer3(message_saturn.begin(), message_saturn.end());
    NS_LOG_INFO("MyRtpsPublisherApp::SendMessageSaturn sends message: " << message_saturn);
    auto change_saturn = m_writer3->newChange(rtps::ChangeKind_t::ALIVE, buffer3.data(), buffer3.size());
    if (change_saturn == nullptr) {
        NS_LOG_ERROR("History full.");
        return;
    }
    m_sendEventSaturn = Simulator::Schedule(MicroSeconds(500.0), &MyRtpsPublisherApp::SendMessageSaturn, this);
}