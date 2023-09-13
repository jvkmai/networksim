#ifndef MYUDP_HELPER_H
#define MYUDP_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "../model/myudpdriver.h"


#include <stdint.h>

namespace ns3
{
/**
 * \ingroup udpclientserver
 * \brief Create a server application which waits for input UDP packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class MyUdpServerHelper
{
  public:

    MyUdpServerHelper();

    /**
     * \param port The port the server will wait on for incoming packets
     */
    MyUdpServerHelper(uint16_t port);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Create one UDP server application on each of the Nodes in the
     * NodeContainer.
     *
     * \param c The nodes on which to create the Applications.  The nodes
     *          are specified by a NodeContainer.
     * \returns The applications created, one Application per Node in the
     *          NodeContainer.
     */
    ApplicationContainer Install(NodeContainer c);

    /**
     * \returns a Ptr to the last created server application
     */
    Ptr<MyUdpDriver> GetServer();

  private:
    ObjectFactory m_factory; //!< Object factory.
    Ptr<MyUdpDriver> m_server; //!< The last created server application
};

/**
 * \ingroup udpclientserver
 * \brief Create a client application which sends UDP packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class MyUdpClientHelper
{
  public:
    /**
     * Create MyUdpClientHelper which will make life easier for people trying
     * to set up simulations with udp-client-server.
     */
    MyUdpClientHelper();

    /**
     * \param ip The IP address of the remote UDP server
     * \param port The port number of the remote UDP server
     */

    MyUdpClientHelper(Address ip, uint16_t port);
    /**
     * \param addr The address of the remote UDP server
     */

    MyUdpClientHelper(Address addr);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * \param c the nodes
     *
     * Create one UDP client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
    ApplicationContainer Install(NodeContainer c);

  private:
    ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* UDP_CLIENT_SERVER_H */
