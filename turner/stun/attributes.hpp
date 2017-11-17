#pragma once

/**
 * \file turner/stun/attributes.hpp
 * STUN protocol attributes
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
  static value_t read (const any_message_t<protocol_traits_t> &message,
    const any_attribute_t &attribute,
    std::error_code &error
  ) noexcept;


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<protocol_traits_t> &message,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error
  ) noexcept;
};


namespace __bits {

// internal type to simplify adding STUN attributes
template <
  uint16_t Type,
  template <typename> typename AttributeProcessor
>
using attribute_type_t = turner::attribute_type_t<
  protocol_traits_t,
  Type,
  AttributeProcessor<protocol_traits_t>
>;

template <uint16_t Type>
using string_type_t = attribute_type_t<Type, string_attribute_processor_t>;

template <uint16_t Type>
using array_type_t = attribute_type_t<Type, array_attribute_processor_t>;

template <uint16_t Type>
using error_type_t = attribute_type_t<Type, error_attribute_processor_t>;

template <uint16_t Type>
using address_type_t = attribute_type_t<Type, address_attribute_processor_t>;

template <uint16_t Type>
using xor_address_type_t = turner::attribute_type_t<
  protocol_traits_t,
  Type,
  xor_address_attribute_processor_t
>;

} // namespace __bits


/**
 * \defgroup STUN_attributes STUN attributes
 * \{
 *
 * \note This library does not implement FINGERPRINT mechanism
 * (https://tools.ietf.org/html/rfc5389#section-8)
 *
 * \see https://tools.ietf.org/html/rfc5389#section-15
 */

/**
 * STUN MAPPED-ADDRESS type (https://tools.ietf.org/html/rfc5389#section-15.1)
 */
using mapped_address_t = __bits::address_type_t<0x0001>;

/**
 * Instance of STUN MAPPED-ADDRESS (https://tools.ietf.org/html/rfc5389#section-15.1)
 */
__turner_inline_var constexpr const mapped_address_t mapped_address;

/**
 * STUN USERNAME type (https://tools.ietf.org/html/rfc5389#section-15.3)
 */
using username_t = __bits::string_type_t<0x0006>;

/**
 * Instance of STUN USERNAME (https://tools.ietf.org/html/rfc5389#section-15.3)
 */
__turner_inline_var constexpr const username_t username;

// 0x0008 message-integrity

/**
 * STUN ERROR-CODE type (https://tools.ietf.org/html/rfc5389#section-15.6)
 */
using error_code_t = __bits::error_type_t<0x0009>;

/**
 * Instance of STUN ERROR-CODE (https://tools.ietf.org/html/rfc5389#section-15.6)
 */
__turner_inline_var constexpr const error_code_t error_code;

/**
 * STUN REALM type (https://tools.ietf.org/html/rfc5389#section-15.7)
 */
using realm_t = __bits::string_type_t<0x0014>;

/**
 * Instance of STUN REALM type (https://tools.ietf.org/html/rfc5389#section-15.7)
 */
__turner_inline_var constexpr const realm_t realm;

/**
 * STUN NONCE type (https://tools.ietf.org/html/rfc5389#section-15.8)
 */
using nonce_t = __bits::array_type_t<0x0015>;

/**
 * Instance of STUN NONCE (https://tools.ietf.org/html/rfc5389#section-15.8)
 */
__turner_inline_var constexpr const nonce_t nonce;

/**
 * STUN XOR-MAPPED-ADDRESS type (https://tools.ietf.org/html/rfc5389#section-15.2)
 */
using xor_mapped_address_t = __bits::xor_address_type_t<0x0020>;

/**
 * Instance of STUN XOR-MAPPED-ADDRESS (https://tools.ietf.org/html/rfc5389#section-15.2)
 */
__turner_inline_var constexpr const xor_mapped_address_t xor_mapped_address;

/**
 * STUN SOFTWARE type (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
using software_t = __bits::string_type_t<0x8022>;

/**
 * Instance of STUN SOFTWARE (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
__turner_inline_var constexpr const software_t software;

/**
 * STUN ALTERNATE-SERVER type (https://tools.ietf.org/html/rfc5389#section-15.11)
 */
using alternate_server_t = __bits::address_type_t<0x8023>;

/**
 * Instance of STUN ALTERNATE-SERVER (https://tools.ietf.org/html/rfc5389#section-15.11)
 */
__turner_inline_var constexpr const alternate_server_t alternate_server;

/// \}


} // namespace stun

__turner_end
