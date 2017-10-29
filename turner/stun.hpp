#pragma once

/**
 * \file turner/stun.hpp
 * STUN protocol (https://tools.ietf.org/html/rfc5389)
 */


#include <turner/config.hpp>
#include <turner/attribute.hpp>
#include <turner/attribute_processor.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <turner/protocol.hpp>


__turner_begin

namespace stun {


/**
 * STUN protocol message layout traits.
 */
struct protocol_traits_t
{
  /**
   * Header size in bytes.
   */
  static __turner_inline_var constexpr const size_t header_size = 20;


  /**
   * Offset from beginning of message to cookie field.
   */
  static __turner_inline_var constexpr const size_t cookie_offset = 4;


  /**
   * Cookie size in bytes.
   */
  static __turner_inline_var constexpr const size_t cookie_size = 4;


  /**
   * Required cookie content.
   */
  static __turner_inline_var constexpr const std::array<uint8_t, cookie_size> cookie =
  {
    { 0x21, 0x12, 0xa4, 0x42 }
  };


  /**
   * Offset from beginning of message to transaction ID field.
   */
  static __turner_inline_var constexpr const size_t transaction_id_offset = 8;


  /**
   * Transaction ID size in bytes.
   */
  static __turner_inline_var constexpr const size_t transaction_id_size = 12;
};


/**
 * Structure type describing STUN protocol message layout.
 */
using protocol_t = basic_protocol_t<protocol_traits_t>;


/**
 * STUN protocol instance.
 */
__turner_inline_var constexpr const protocol_t protocol{};


/**
 * Return STUN protocol \a name in output argument.
 */
inline constexpr void operator>> (protocol_t, const char *&name) noexcept
{
  name = "STUN";
}


/**
 * \defgroup STUN_methods STUN methods
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5389#section-18.1
 */

/**
 * STUN Binding message type.
 */
using binding_t = protocol_t::message_type_t<0x001>;

/**
 * Instance of STUN Binding message type.
 */
__turner_inline_var constexpr const binding_t binding{};

/**
 * STUN Binding success response message type
 */
using binding_success_t = binding_t::success_response_t;

/**
 * Instance of STUN Binding success resposen message type.
 */
__turner_inline_var constexpr const binding_success_t binding_success;

// \}

/**
 * Return STUN Binding message \a name in output argument.
 */
inline constexpr void operator>> (binding_t, const char *&name) noexcept
{
  name = "binding";
}

/**
 * Return STUN Binding success response message \a name in output argument.
 */
inline constexpr void operator>> (binding_success_t, const char *&name) noexcept
{
  name = "binding_success";
}


//
// STUN attributes
//


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
   * Read \a attribute value. On failure return default value and set \a error
   * to code describing failure reason.
   */
  static value_t read (
    const any_message_t<protocol_traits_t> &message,
    const any_attribute_t &attribute,
    std::error_code &error
  ) noexcept;
};


namespace __bits {

// internal type to simplify adding STUN attributes
template <
  uint16_t Type,
  template <typename> typename AttributeProcessor
>
using attribute_type_t = basic_attribute_type_t<
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
using xor_address_type_t = basic_attribute_type_t<
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


/**
 * \defgroup STUN_errors STUN errors
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5389#section-15.6
 */

/**
 * Client should contact an alternate server for this request.
 */
__turner_inline_var constexpr const error_t try_alternate{
  300, "Try Alternate"
};

/**
 * The request was malformed.
 */
__turner_inline_var constexpr const error_t bad_request{
  400, "Bad Request"
};

/**
 * The request did not contain the correct credentials to proceed.
 */
__turner_inline_var constexpr const error_t unauthorized{
  401, "Unauthorized"
};

/**
 * The server received a STUN packet containing comprehension-required
 * attribute that it did not understand.
 */
__turner_inline_var constexpr const error_t unknown_attribute{
  420, "Unknown Attribute"
};

/**
 * The NONCE used by the client was no longer valid.
 */
__turner_inline_var constexpr const error_t stale_nonce{
  438, "Stale Nonce"
};

/**
 * The server has suffered a temporary error. The client should try again.
 */
__turner_inline_var constexpr const error_t server_error{
  500, "Server Error"
};

/// \}


} // namespace stun

__turner_end
