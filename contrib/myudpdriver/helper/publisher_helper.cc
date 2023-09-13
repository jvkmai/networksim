#include "publisher_helper.h"
#include "ns3/app-publisher.h"

namespace ns3 {

PublisherAppHelper::PublisherAppHelper () {
    m_factory.SetTypeId (MyRtpsPublisherApp::GetTypeId());
}

PublisherAppHelper::PublisherAppHelper(Address address, uint16_t port) {
    m_factory.SetTypeId (MyRtpsPublisherApp::GetTypeId());
    SetAttribute("LocalAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

PublisherAppHelper::PublisherAppHelper(Address address) {
    m_factory.SetTypeId (MyRtpsPublisherApp::GetTypeId());
    SetAttribute("LocalAddress", AddressValue(address));
}

void PublisherAppHelper::SetAttribute (std::string name, const AttributeValue &value) {
    m_factory.Set (name, value);
}

ApplicationContainer 
PublisherAppHelper::Install (NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<MyRtpsPublisherApp> app = m_factory.Create<MyRtpsPublisherApp> ();
        node->AddApplication (app);
        apps.Add (app);
    }
    return apps;
}

} // namespace ns3
