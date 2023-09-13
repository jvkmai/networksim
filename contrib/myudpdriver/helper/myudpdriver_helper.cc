#include "myudpdriver_helper.h"
#include "../model/myudpdriver.h"

#include "ns3/string.h"
#include "ns3/uinteger.h"

namespace ns3
{

MyUdpServerHelper::MyUdpServerHelper()
{
    m_factory.SetTypeId(MyUdpDriver::GetTypeId());
}

MyUdpServerHelper::MyUdpServerHelper(uint16_t port)
{
    m_factory.SetTypeId(MyUdpDriver::GetTypeId());
    SetAttribute("LocalPort", UintegerValue(port));
}

void
MyUdpServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
MyUdpServerHelper::Install(NodeContainer c)
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;

        m_server = m_factory.Create<MyUdpDriver>();
        node->AddApplication(m_server);
        apps.Add(m_server);
    }
    return apps;
}

Ptr<MyUdpDriver>
MyUdpServerHelper::GetServer()
{
    return m_server;
}

MyUdpClientHelper::MyUdpClientHelper()
{
    m_factory.SetTypeId(MyUdpDriver::GetTypeId());
}

MyUdpClientHelper::MyUdpClientHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId(MyUdpDriver::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

MyUdpClientHelper::MyUdpClientHelper(Address address)
{
    m_factory.SetTypeId(MyUdpDriver::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
}

void
MyUdpClientHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
MyUdpClientHelper::Install(NodeContainer c)
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<MyUdpDriver> client = m_factory.Create<MyUdpDriver>();
        node->AddApplication(client);
        apps.Add(client);
    }
    return apps;
}
} // namespace ns3
