#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>

using namespace ns3;

class VLANTag : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // these are our accessors to our tag structure
  /**
   * Set the tag value
   * \param value The tag value.
   */
  void SetVlanTag (uint8_t value);
  /**
   * Get the tag value
   * \return the tag value.
   */
  uint8_t GetVlanTag (void) const;
private:
  uint8_t m_vlantag;  //!< tag value
};

TypeId 
VLANTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VLANTag")
    .SetParent<Tag> ()
    .AddConstructor<VLANTag> ()
    .AddAttribute ("VlanTag",
                   "A VlanTag",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&VLANTag::GetVlanTag),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}
TypeId 
VLANTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
VLANTag::GetSerializedSize (void) const
{
  return 1;
}
void 
VLANTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_vlantag);
}
void 
VLANTag::Deserialize (TagBuffer i)
{
  m_vlantag = i.ReadU8 ();
}
void 
VLANTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_vlantag;
}
void 
VLANTag::SetVlanTag (uint8_t value)
{
  m_vlantag = value;
}
uint8_t 
VLANTag::GetVlanTag (void) const
{
  return m_vlantag;
}

