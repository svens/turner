#pragma once

/**
 * \file turner/turn/attributes.hpp
 * TURN protocol attributes.
 */


#include <turner/config.hpp>
#include <turner/turn/protocol.hpp>
#include <turner/stun/attributes.hpp>
#include <turner/__bits/attribute_processor.hpp>


__turner_begin


namespace turn {


/**
 * \defgroup TURN_attributes TURN attributes
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5766#section-14
 *
 * \note This library does not implement following attributes:
 *   - EVEN-PORT (https://tools.ietf.org/html/rfc5766#section-14.6)
 *   - DONT-FRAGMENT (https://tools.ietf.org/html/rfc5766#section-14.8)
 *   - RESERVATION-TOKEN (https://tools.ietf.org/html/rfc5766#section-14.9)
 */


// 0x000c CHANNEL-NUMBER {{{1


/**
 * TURN attribute CHANNEL-NUMBER reader/writer.
 */
template <typename ProtocolTraits>
struct channel_number_attribute_processor_t
{
  /**
   * \copydoc uint32_attribute_processor_t::value_t
   */
  using value_t = uint16_t;

  /**
   * Minimum valid value for CHANNEL-NUMBER.
   */
  static constexpr const value_t min_channel_number = 0x4000;

  /**
   * Maximum valid value for CHANNEL-NUMBER.
   */
  static constexpr const value_t max_channel_number = 0x7fff;

  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error
  ) noexcept;

  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error
  ) noexcept;
};


/**
 * TURN CHANNEL-NUMBER attribute type
 * (https://tools.ietf.org/html/rfc5766#section-14.1)
 */
using channel_number_t = protocol_t::attribute_type_t<0x000c,
  channel_number_attribute_processor_t
>;


/**
 * TURN CHANNEL-NUMBER attribute
 * (https://tools.ietf.org/html/rfc5766#section-14.1)
 */
inline constexpr const channel_number_t channel_number;


// 0x000d LIFETIME {{{1


/**
 * TURN LIFETIME attribute type
 * (https://tools.ietf.org/html/rfc5766#section-14.2)
 */
using lifetime_t = protocol_t::attribute_type_t<0x000d,
  seconds_attribute_processor_t
>;


/**
 * TURN LIFETIME attribute
 * (https://tools.ietf.org/html/rfc5766#section-14.2)
 */
inline constexpr const lifetime_t lifetime;


// 0x0012 XOR-PEER-ADDRESS {{{1


/**
 * TURN XOR-PEER-ADDRESS attribute type
 * (https://tools.ietf.org/html/rfc5766#section-14.3)
 */
using xor_peer_address_t = protocol_t::attribute_type_t<0x0012,
  stun::xor_address_attribute_processor_t
>;


/**
 * TURN XOR-PEER-ADDRESS attribute
 * (https://tools.ietf.org/html/rfc5766#section-14.3)
 */
inline constexpr const xor_peer_address_t xor_peer_address;


// 0x0013 DATA {{{1


/**
 * TURN DATA attribute type
 * (https://tools.ietf.org/html/rfc5766#section-14.4)
 */
using data_t = protocol_t::attribute_type_t<0x0013,
  array_attribute_processor_t
>;


/**
 * TURN DATA attribute
 * (https://tools.ietf.org/html/rfc5766#section-14.4)
 */
inline constexpr const data_t data;


// 0x0016 XOR-RELAYED-ADDRESS {{{1


/**
 * TURN XOR-RELAYED-ADDRESS attribute type
 * (https://tools.ietf.org/html/rfc5766#section-14.5)
 */
using xor_relayed_address_t = protocol_t::attribute_type_t<0x0016,
  stun::xor_address_attribute_processor_t
>;


/**
 * TURN XOR-RELAYED-ADDRESS attribute
 * (https://tools.ietf.org/html/rfc5766#section-14.5)
 */
inline constexpr const xor_relayed_address_t xor_relayed_address;


// 0x0017 REQUESTED-ADDRESS-FAMILY {{{1


/**
 * TURN REQUESTED-ADDRESS-FAMILY attribute type
 * (https://tools.ietf.org/html/rfc6156#section-4.1.1)
 */
using requested_address_family_t = protocol_t::attribute_type_t<0x0017,
  address_family_attribute_processor_t
>;


/**
 * TURN REQUESTED-ADDRESS-FAMILY attribute
 * (https://tools.ietf.org/html/rfc6156#section-4.1.1)
 */
inline constexpr const requested_address_family_t requested_address_family;


// 0x0019 REQUESTED-TRANSPORT {{{1


/**
 * Transport protocol numbers for REQUESTED-TRANSPORT
 * \see https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
 */
enum class transport_protocol_t: uint8_t
{
  tcp = 6,
  udp = 17,
};


/**
 * TURN attribute REQUESTED-TRANSPORT reader/writer.
 */
template <typename ProtocolTraits>
struct transport_protocol_attribute_processor_t
{
  /**
   * \copydoc uint32_attribute_processor_t::value_t
   */
  using value_t = transport_protocol_t;

  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    auto value = __bits::read_uint32(attribute, error);
    if (!error)
    {
      value = (value & 0xff00'0000) >> 24;
    }
    return static_cast<value_t>(value);
  }

  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    return __bits::write_uint32(first, last,
      (static_cast<uint32_t>(value) << 24) & 0xff00'0000,
      error
    );
  }
};


/**
 * TURN REQUESTED-TRANSPORT attribute type
 * (https://tools.ietf.org/html/rfc5766#section-14.7)
 */
using requested_transport_t = protocol_t::attribute_type_t<0x0019,
  transport_protocol_attribute_processor_t
>;


/**
 * TURN REQUESTED-TRANSPORT attribute
 * (https://tools.ietf.org/html/rfc5766#section-14.7)
 */
inline constexpr const requested_transport_t requested_transport;


// STUN MAPPED-ADDRESS {{{1


/**
 * STUN MAPPED-ADDRESS attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.1)
 */
using mapped_address_t = protocol_t::reuse_attribute_t<stun::mapped_address_t>;


/**
 * STUN MAPPED-ADDRESS attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.1)
 */
inline constexpr const mapped_address_t mapped_address;


// STUN USERNAME {{{1


/**
 * STUN USERNAME attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.3)
 */
using username_t = protocol_t::reuse_attribute_t<stun::username_t>;


/**
 * STUN USERNAME attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.3)
 */
inline constexpr const username_t username;


// STUN ERROR-CODE {{{1


/**
 * STUN ERROR-CODE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.6)
 */
using error_code_t = protocol_t::reuse_attribute_t<stun::error_code_t>;


/**
 * STUN ERROR-CODE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.6)
 */
inline constexpr const error_code_t error_code;


// STUN REALM {{{1


/**
 * STUN REALM attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.7)
 */
using realm_t = protocol_t::reuse_attribute_t<stun::realm_t>;


/**
 * STUN REALM attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.7)
 */
inline constexpr const realm_t realm;


// STUN NONCE {{{1


/**
 * STUN NONCE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.8)
 */
using nonce_t = protocol_t::reuse_attribute_t<stun::nonce_t>;


/**
 * STUN NONCE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.8)
 */
inline constexpr const nonce_t nonce;


// STUN XOR-MAPPED-ADDRESS {{{1


/**
 * STUN XOR-MAPPED-ADDRESS attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.2)
 */
using xor_mapped_address_t = protocol_t::reuse_attribute_t<stun::xor_mapped_address_t>;


/**
 * STUN XOR-MAPPED-ADDRESS attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.2)
 */
inline constexpr const xor_mapped_address_t xor_mapped_address;


// STUN SOFTWARE {{{1


/**
 * STUN SOFTWARE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
using software_t = protocol_t::reuse_attribute_t<stun::software_t>;


/**
 * STUN SOFTWARE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
inline constexpr const software_t software;


// STUN ALTERNATE-SERVER {{{1


/**
 * STUN ALTERNATE-SERVER attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.11)
 */
using alternate_server_t = protocol_t::reuse_attribute_t<stun::alternate_server_t>;


/**
 * STUN ALTERNATE-SERVER attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.11)
 */
inline constexpr const alternate_server_t alternate_server;


// }}}1


/// \}


template <typename ProtocolTraits>
auto channel_number_attribute_processor_t<ProtocolTraits>::read (
  const any_message_t<ProtocolTraits> &,
  const any_attribute_t &attribute,
  std::error_code &error) noexcept -> value_t
{
  auto value = __bits::read_uint32(attribute, error);
  if (!error)
  {
    value = (value & 0xffff'0000) >> 16;
    if (min_channel_number <= value && value <= max_channel_number)
    {
      return static_cast<value_t>(value);
    }
    error = make_error_code(errc::unexpected_attribute_value);
  }
  return {};
}


template <typename ProtocolTraits>
size_t channel_number_attribute_processor_t<ProtocolTraits>::write (
  const any_message_t<ProtocolTraits> &,
  uint8_t *first, uint8_t *last,
  const value_t &value,
  std::error_code &error) noexcept
{
  if (min_channel_number <= value && value <= max_channel_number)
  {
    return __bits::write_uint32(first, last,
      (static_cast<uint32_t>(value) << 16) & 0xffff'0000,
      error
    );
  }
  error = make_error_code(errc::unexpected_attribute_value);
  return sizeof(uint32_t);
}


} // namespace turn


__turner_end
