#ifndef UDP_DRIVER_HELPER_H
#define UDP_DRIVER_HELPER_H

#include "ns3/rtps-udpdriver.h"
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3 {

class UdpDriverHelper
{
public:
    UdpDriverHelper ();

    UdpDriverHelper (Address remoteAddress, uint16_t remotePort);

    void SetAttribute (std::string name, const AttributeValue &value);

    ApplicationContainer Install (NodeContainer c) const;

private:

    ObjectFactory m_factory;
};

} // namespace ns3

#endif /* UDP_DRIVER_HELPER_H */
