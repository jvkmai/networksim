#include "RTPS-UdpDriverHelper.h"
#include "ns3/rtps-udpdriver.h"
#include "ns3/names.h"
#include "ns3/ipv4-address.h"
#include "ns3/uinteger.h"

namespace ns3 {

UdpDriverHelper::UdpDriverHelper ()
{
  m_factory.SetTypeId ("ns3::UdpDriver");
}

UdpDriverHelper::UdpDriverHelper (Address remoteAddress, uint16_t remotePort)
{
  m_factory.SetTypeId ("ns3::UdpDriver");
  m_factory.Set ("RemoteAddress", AddressValue (remoteAddress));
  m_factory.Set ("RemotePort", UintegerValue (remotePort));
}

void 
UdpDriverHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpDriverHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<UdpDriver> driver = m_factory.Create<UdpDriver>();
        node->AddApplication(driver);
        apps.Add(driver);
    }
    return apps;
}


} // namespace ns3
