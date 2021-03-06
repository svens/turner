#pragma once

/**
 * \file turner/attribute.hpp
 * STUN/TURN/MSTURN protocol attribute structure.
 */


#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>
#include <utility>


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
   * Return true if attribute's type falls into comprehension-required range.
   */
  bool is_comprehension_required () const noexcept
  {
    return type() < 0x8000;
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
   * Return pointer to attribute's value data area.
   */
  const uint8_t *data () const noexcept
  {
    return reinterpret_cast<const uint8_t *>(this) + 2 * sizeof(uint16_t);
  }


  /**
   * Return pointer to first byte of attribute's value data area.
   */
  const uint8_t *begin () const noexcept
  {
    return data();
  }


  /**
   * Return pointer to one past last byte of attribute's value data area.
   */
  const uint8_t *end () const noexcept
  {
    return data() + length();
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
  template <typename> typename AttributeProcessor
>
class attribute_type_t
{
public:

  /**
   * Protocol traits.
   */
  using traits_t = ProtocolTraits;

  /**
   * Protocol class describing raw network message format traits.
   */
  using protocol_t = turner::protocol_t<traits_t>;


  /**
   * Concrete \a AttributeType reader/writer.
   */
  using processor_t = AttributeProcessor<traits_t>;


  /**
   * \a AttributeType value type.
   */
  using value_t = typename processor_t::value_t;


  /**
   * \a AttributeType value in attributes' registry.
   */
  static constexpr uint16_t type = AttributeType;


  /**
   * Pair containing attribute type and const reference to it's value.
   */
  using value_cref_t = std::pair<attribute_type_t, const value_t &>;


  /**
   * Return pair containing attribute type and const reference to it's
   * \a value.
   */
  value_cref_t value_cref (const value_t &value) const noexcept
  {
    return {*this, value};
  }


  /**
   * Pair containing attribute type and mutable reference to it's value.
   */
  using value_ref_t = std::pair<attribute_type_t, value_t &>;


  /**
   * Return pair containing attribute type and mutable reference to it's
   * \a value.
   */
  value_ref_t value_ref (value_t &value) const noexcept
  {
    return {*this, value};
  }


  /**
   * Helper for protocol_t::reuse_attribute_t to extract basic
   * AttributeProcessor without ProtocolTraits for rebinding with different
   * traits.
   *
   * \internal
   */
  template <typename OtherProtocolTraits>
  using rebind_processor_t = AttributeProcessor<OtherProtocolTraits>;
};


__turner_end
