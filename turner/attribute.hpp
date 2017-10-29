#pragma once

/**
 * \file turner/attribute.hpp
 * STUN/TURN/MSTURN protocol attribute structure.
 */


#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>


__turner_begin


/**
 * Generic STUN-based protocol attribute encoded as TLV (Type-Length-Value).
 * Encoding is documented at https://tools.ietf.org/html/rfc5389#section-15.
 * TURN and MSTURN are encoded same way.
 */
class any_attribute_t
{
public:

  /**
   * Attribute type (in host byte order).
   */
  uint16_t type () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(this)[0]
    );
  }


  /**
   * Attribute length (without 4B boundary padding)
   */
  uint16_t length () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(this)[1]
    );
  }


  /**
   * Attribute size in bytes over all TLV fields and possible padding.
   */
  size_t size () const noexcept
  {
    return 2 * sizeof(uint16_t) + ((length() + 3) & ~3);
  }


  /**
   * Return pointer to attribute's value data area.
   */
  const uint8_t *data () const noexcept
  {
    return reinterpret_cast<const uint8_t *>(this) + 2 * sizeof(uint16_t);
  }


  /**
   * Pointer to next attribute.
   */
  const any_attribute_t *next () const noexcept
  {
    return reinterpret_cast<const any_attribute_t *>(
      __bits::to_ptr(this) + size()
    );
  }


private:

  any_attribute_t () = delete;
  any_attribute_t (const any_attribute_t &) = delete;
  any_attribute_t &operator= (const any_attribute_t &) = delete;
  any_attribute_t (any_attribute_t &&) = delete;
  any_attribute_t &operator= (any_attribute_t &&) = delete;
};


/**
 * Attribute value reader/writer.
 */
template <
  typename ProtocolTraits,
  uint16_t AttributeType,
  typename AttributeProcessor
>
class basic_attribute_type_t
{
public:

  /**
   * Protocol class describing raw network message format traits.
   */
  using protocol_t = basic_protocol_t<ProtocolTraits>;


  /**
   * Concrete \a AttributeType reader/writer.
   */
  using processor_t = AttributeProcessor;


  /**
   * \a AttributeType value type.
   */
  using value_t = typename processor_t::value_t;


  /**
   * \a AttributeType value in attributes' registry.
   */
  static constexpr uint16_t type () noexcept
  {
    return AttributeType;
  }
};


__turner_end
