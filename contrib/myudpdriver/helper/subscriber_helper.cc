#include "subscriber_helper.h"
#include "ns3/app-subscriber.h"

namespace ns3 {

SubscriberAppHelper::SubscriberAppHelper () {
    m_factory.SetTypeId (MyRtpsSubscriberApp::GetTypeId());
}

SubscriberAppHelper::SubscriberAppHelper(Address address) {
    m_factory.SetTypeId (MyRtpsSubscriberApp::GetTypeId());
    SetAttribute("LocalAddress", AddressValue(address));
}

SubscriberAppHelper::SubscriberAppHelper(Address address, uint16_t type) {
    m_factory.SetTypeId (MyRtpsSubscriberApp::GetTypeId());
    SetAttribute("LocalAddress", AddressValue(address));
    SetAttribute("ReceiveType", UintegerValue(type));
}

void SubscriberAppHelper::SetAttribute (std::string name, const AttributeValue &value) {
    m_factory.Set (name, value);
}

ApplicationContainer 
SubscriberAppHelper::Install (NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<MyRtpsSubscriberApp> app = m_factory.Create<MyRtpsSubscriberApp> ();
        node->AddApplication (app);
        apps.Add (app);
    }
    return apps;
}

} // namespace ns3
