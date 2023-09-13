#ifndef SUBSCRIBER_APP_HELPER_H
#define SUBSCRIBER_APP_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"

namespace ns3 {

class SubscriberAppHelper
{
public:
  SubscriberAppHelper ();
  SubscriberAppHelper (Address address);
  SubscriberAppHelper (Address address, uint16_t type);

  void SetAttribute (std::string name, const AttributeValue &value);

  ApplicationContainer Install (NodeContainer c) const;

private:
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* SUBSCRIBER_APP_HELPER_H */
