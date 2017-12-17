#pragma once

/**
 * \file turner/stun/attributes.hpp
 * STUN protocol attributes.
 */


#include <turner/config.hpp>
#include <turner/attribute.hpp>
#include <turner/attribute_processor.hpp>
#include <turner/stun/protocol.hpp>


__turner_begin

namespace stun {


/**
 * Generic XORed address type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct xor_address_attribute_processor_t
{
  /**
   * Attribute value type.
   * - first: address
   * - second: port in host byte order
   */
  using value_t = std::pair<sal::net::ip::address_t, uint16_t>;


  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &message,
    const any_attribute_t &attribute,
    std::error_code &error
  ) noexcept;


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &message,
    uint8_t *first, uint8_t *last,
    value_t value,
    std::error_code &error
  ) noexcept;


private:

  static constexpr const uint16_t xor_cookie_16 = 0x2112;
  static constexpr const uint32_t xor_cookie_32 = 0x2112a442;
};


/**
 * \defgroup STUN_attributes STUN attributes
 * \{
 *
 * \note This library does not implement FINGERPRINT mechanism
 * (https://tools.ietf.org/html/rfc5389#section-8)
 *
 * \see https://tools.ietf.org/html/rfc5389#section-15
 */


// 0x0001 MAPPED-ADDRESS {{{1


/**
 * STUN MAPPED-ADDRESS attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.1)
 */
using mapped_address_t = protocol_t::attribute_type_t<0x0001,
  address_attribute_processor_t
>;


/**
 * STUN MAPPED-ADDRESS attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.1)
 */
inline constexpr const mapped_address_t mapped_address;


// 0x0006 USERNAME {{{1


/**
 * STUN USERNAME attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.3)
 */
using username_t = protocol_t::attribute_type_t<0x0006,
  string_attribute_processor_t
>;


/**
 * STUN USERNAME attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.3)
 */
inline constexpr const username_t username;


// 0x0009 ERROR-CODE {{{1


/**
 * STUN ERROR-CODE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.6)
 */
using error_code_t = protocol_t::attribute_type_t<0x0009,
  error_attribute_processor_t
>;


/**
 * STUN ERROR-CODE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.6)
 */
inline constexpr const error_code_t error_code;


// 0x0014 REALM {{{1


/**
 * STUN REALM attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.7)
 */
using realm_t = protocol_t::attribute_type_t<0x0014,
  string_attribute_processor_t
>;


/**
 * STUN REALM attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.7)
 */
inline constexpr const realm_t realm;


// 0x0015 NONCE {{{1


/**
 * STUN NONCE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.8)
 */
using nonce_t = protocol_t::attribute_type_t<0x0015,
  array_attribute_processor_t
>;


/**
 * STUN NONCE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.8)
 */
inline constexpr const nonce_t nonce;


// 0x0020 XOR-MAPPED-ADDRESS {{{1


/**
 * STUN XOR-MAPPED-ADDRESS attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.2)
 */
using xor_mapped_address_t = protocol_t::attribute_type_t<0x0020,
  xor_address_attribute_processor_t
>;


/**
 * STUN XOR-MAPPED-ADDRESS attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.2)
 */
inline constexpr const xor_mapped_address_t xor_mapped_address;


// 0x8022 SOFTWARE {{{1


/**
 * STUN SOFTWARE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
using software_t = protocol_t::attribute_type_t<0x8022,
  string_attribute_processor_t
>;


/**
 * STUN SOFTWARE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
inline constexpr const software_t software;


// 0x8023 ALTERNATE-SERVER {{{1


/**
 * STUN ALTERNATE-SERVER attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.11)
 */
using alternate_server_t = protocol_t::attribute_type_t<0x8023,
  address_attribute_processor_t
>;


/**
 * STUN ALTERNATE-SERVER attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.11)
 */
inline constexpr const alternate_server_t alternate_server;


// }}}1


/// \}


template <typename ProtocolTraits>
typename xor_address_attribute_processor_t<ProtocolTraits>::value_t
  xor_address_attribute_processor_t<ProtocolTraits>::read (
    const any_message_t<ProtocolTraits> &message,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
{
  auto [ address, port ] = address_attribute_processor_t<ProtocolTraits>::read(
    message, attribute, error
  );
  if (!error)
  {
    port ^= xor_cookie_16;

    // IPv4
    if (auto v4 = address.as_v4())
    {
      address = sal::net::ip::make_address_v4(v4->to_uint() ^ xor_cookie_32);
    }

    // IPv6
    else if (auto v6 = address.as_v6())
    {
      auto p = const_cast<uint8_t *>(v6->to_bytes().data());
      for (auto b: ProtocolTraits::cookie)
      {
        *p++ ^= b;
      }
      for (auto b: message.transaction_id())
      {
        *p++ ^= b;
      }
    }

    return { address, port };
  }
  return {};
}


template <typename ProtocolTraits>
size_t xor_address_attribute_processor_t<ProtocolTraits>::write (
  const any_message_t<ProtocolTraits> &message,
  uint8_t *first, uint8_t *last,
  value_t value,
  std::error_code &error) noexcept
{
  value.second ^= xor_cookie_16;

  // IPv4
  if (auto v4 = value.first.as_v4())
  {
    value.first = sal::net::ip::make_address_v4(
      v4->to_uint() ^ xor_cookie_32
    );
  }

  // IPv6
  else if (auto v6 = value.first.as_v6())
  {
    auto p = const_cast<uint8_t *>(v6->to_bytes().data());
    for (auto b: ProtocolTraits::cookie)
    {
      *p++ ^= b;
    }
    for (auto b: message.transaction_id())
    {
      *p++ ^= b;
    }
  }

  return turner::__bits::write_address(first, last, value, error);
}


} // namespace stun

__turner_end
