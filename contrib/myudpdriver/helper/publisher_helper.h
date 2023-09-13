#ifndef PUBLISHER_APP_HELPER_H
#define PUBLISHER_APP_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"

namespace ns3 {

class PublisherAppHelper
{
public:
  PublisherAppHelper ();
  PublisherAppHelper (Address address, uint16_t port);
  PublisherAppHelper (Address address);

  void SetAttribute (std::string name, const AttributeValue &value);
  
  ApplicationContainer Install (NodeContainer c) const;

private:
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* PUBLISHER_APP_HELPER_H */
