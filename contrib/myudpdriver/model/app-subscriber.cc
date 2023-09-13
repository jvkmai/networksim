#include "app-subscriber.h"

#include "rtps/communication/UdpDriver.h"
#include "rtps/communication/TcpipCoreLock.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

#include <lwip/igmp.h>
#include <lwip/tcpip.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyRtpsSubscriberApp");

NS_OBJECT_ENSURE_REGISTERED (MyRtpsSubscriberApp);

TypeId
MyRtpsSubscriberApp::GetTypeId(void)
{
  static TypeId tid = TypeId ("ns3::MyRtpsSubscriberApp")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<MyRtpsSubscriberApp>()
    .AddAttribute ("LocalAddress", "The local Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&MyRtpsSubscriberApp::m_localAddress),
                   MakeAddressChecker ())
    .AddAttribute ("TopicName", "Name of the topic to subscribe to",
                   StringValue (""),
                   MakeStringAccessor (&MyRtpsSubscriberApp::m_topicName),
                   MakeStringChecker ())
    .AddAttribute ("TopicType", "Type of the topic to subscribe to",    
                   StringValue (""),
                   MakeStringAccessor (&MyRtpsSubscriberApp::m_topicType),
                   MakeStringChecker ());
  return tid;
}

TypeId
MyRtpsSubscriberApp::GetInstanceTypeId(void) const
{
  return MyRtpsSubscriberApp::GetTypeId();
}

MyRtpsSubscriberApp::MyRtpsSubscriberApp() {
    NS_LOG_FUNCTION(this);
}

MyRtpsSubscriberApp::~MyRtpsSubscriberApp() {
    NS_LOG_FUNCTION(this);
}

void MyRtpsSubscriberApp::StartApplication() {
    NS_LOG_FUNCTION(this);

    uint32_t ipv4Addr = ns3::Ipv4Address::ConvertFrom(m_localAddress).Get();
    std::array<uint8_t, 4> ipBytes;
    ipBytes[0] = static_cast<uint8_t>((ipv4Addr >> 24) & 0xFF);
    ipBytes[1] = static_cast<uint8_t>((ipv4Addr >> 16) & 0xFF);
    ipBytes[2] = static_cast<uint8_t>((ipv4Addr >> 8) & 0xFF);
    ipBytes[3] = static_cast<uint8_t>(ipv4Addr & 0xFF);
    IP_ADDRESS = ipBytes;

    Ptr<Node> node = GetNode();
    NS_LOG_INFO("[MyRtpsSubscriberApp::StartApplication] creates UdpDriver on node: " << node);
    m_factory.SetTypeId ("ns3::UdpDriver");
    Ptr<UdpDriver> driver = m_factory.Create<UdpDriver>();
    node->AddApplication(driver);
    m_domain = std::make_unique<rtps::Domain>(*driver, IP_ADDRESS);
    m_participant = m_domain->createParticipant(node->GetId());
    m_domain->createBuiltinWritersAndReaders(*m_participant, IP_ADDRESS);
    if(m_participant == nullptr){
        NS_FATAL_ERROR("Failed to create participant");
    }
    NS_LOG_INFO("[MyRtpsSubscriberApp::StartApplication] creates Reader");
    m_reader = m_domain->createReader(*m_participant, m_topicName.c_str(), m_topicType.c_str(), false);
    if (m_reader == nullptr) {
        NS_FATAL_ERROR("Failed to create Reader");
    }  
    m_domain->completeInit();
    m_participant->getSPDPAgent().start();
    m_reader->registerCallback(&MyRtpsSubscriberApp::ReceiveCallback, this);
    m_timeData.reserve(100000);
}

void MyRtpsSubscriberApp::StopApplication() {
    NS_LOG_FUNCTION(this);
    m_domain->stop();
    WriteTimingDataToTextFile();
}

void MyRtpsSubscriberApp::ReceiveCallback(void* callee, const rtps::ReaderCacheChange& cacheChange) {
    MyRtpsSubscriberApp* instance = static_cast<MyRtpsSubscriberApp*>(callee);

    std::array<uint8_t, 256> buffer;
    bool success = cacheChange.copyInto(buffer.data(), buffer.size());
    if(success){
        std::string message(reinterpret_cast<char*>(buffer.data()));

        std::string searchString = "Message ID: ";
        size_t pos = message.find(searchString);
        int msgID = -1;
        if (pos != std::string::npos) {
            pos += searchString.size();
            size_t endPos = message.find(' ', pos);
            std::string idString = message.substr(pos, endPos - pos);
            try {
                msgID = std::stoi(idString);
            } catch(const std::invalid_argument& e) {
                std::cerr << "Cannot convert msgID to integer: " << e.what() << '\n';
            }
        } else {
        std::cerr << "Cannot find 'Message ID: ' in the message." << '\n';
        }

        searchString = "Creation Time: ";
        pos = message.find(searchString);
        int64_t sendTime = 0;
        if (pos != std::string::npos) {
            pos += searchString.size();
            size_t endPos = message.find(' ', pos);
            std::string idString = message.substr(pos, endPos - pos);
            try {
                sendTime = std::stoll(idString);
            } catch(const std::invalid_argument& e) {
                std::cerr << "Cannot convert sendTime to integer: " << e.what() << '\n';
            }
        } else {
        std::cerr << "Cannot find 'Creation Time: ' in the message." << '\n';
        }


        int64_t receiveTime = Simulator::Now().GetMicroSeconds();
        int64_t delay = receiveTime - sendTime;

        instance->m_timeData.push_back({sendTime, receiveTime, delay, msgID});
        
        NS_LOG_INFO("Received message: " << reinterpret_cast<char*>(buffer.data()));
    } 
    else {
        NS_LOG_ERROR("Failed to copy the received message into the buffer");
    }
}

void MyRtpsSubscriberApp::WriteTimingDataToTextFile() {
    uint32_t nodeId = GetNode()->GetId();
    std::string filename = "timingData" + std::to_string(nodeId) + ".csv";
    std::string overviewFilename = "timingOverview" + std::to_string(nodeId) + ".csv";

    std::ofstream file;
    std::ofstream overviewFile;
    file.open(filename, std::ios::out);
    overviewFile.open(overviewFilename, std::ios::out);

    if (!file) {
        NS_LOG_ERROR("Unable to open file: " << filename);
        return;
    }

    if (!overviewFile) {
        NS_LOG_ERROR("Unable to open file: " << overviewFilename);
        return;
    }

    file << "Message ID, CreationTime [μs]  , ReceiveTime [μs] , TimeDifference [μs]\n";

    double sumSendTimeDiffs = 0.0;
    double sumFirst10SendTimeDiffs = 0.0;
    double sumTimeDiffs = 0.0;
    double maxTimeDiff = 0.0;
    int droppedPackages = 0;
    int prevMsgId = -1; //Message IDs start at 0
    double prevSendTime = 0.0;

    for (size_t i = 0; i < m_timeData.size(); i++) {
        const auto& data = m_timeData[i];

        if (i != 0) { // skip the first entry
            double sendTimeDiff = data.sendTime - prevSendTime;
            sumSendTimeDiffs += sendTimeDiff;
            if (i <= 10) { // for the first 10 packages
                sumFirst10SendTimeDiffs += sendTimeDiff;
            }
        }
        prevSendTime = data.sendTime;

        sumTimeDiffs += data.timeDiff;

        if (data.timeDiff > maxTimeDiff) {
            maxTimeDiff = data.timeDiff;
        }

        if (data.msgId != prevMsgId + 1) {
            droppedPackages += data.msgId - prevMsgId - 1;
        }
        prevMsgId = data.msgId;

        file << data.msgId << ", " 
             << data.sendTime << ", " 
             << data.receiveTime << ", " 
             << data.timeDiff << "\n";
    }

    double averageSendTimeDiff = m_timeData.size() > 1 ? sumSendTimeDiffs / (m_timeData.size() - 1) : 0;
    double averageFirst10SendTimeDiff = m_timeData.size() > 1 ? sumFirst10SendTimeDiffs / std::min((size_t)10, m_timeData.size() - 1) : 0;
    double averageTimeDiff = sumTimeDiffs / m_timeData.size();

    overviewFile << "Average send time between first 10 packages [μs]," << averageFirst10SendTimeDiff << "\n";
    overviewFile << "Average send time between packages [μs]," << averageSendTimeDiff << "\n";
    overviewFile << "Average receive time difference [μs]," << averageTimeDiff << "\n";
    overviewFile << "Longest receive time difference [μs]," << maxTimeDiff << "\n";
    overviewFile << "Dropped packages," << droppedPackages << "\n";
    overviewFile << "Total packages," << m_timeData.size() << "\n";

    file.close();
    overviewFile.close();
}
