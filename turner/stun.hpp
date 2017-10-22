#pragma once

/**
 * \file turner/stun.hpp
 * STUN protocol (RFC5389)
 */


#include <turner/config.hpp>
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
 * Return STUN protocol \a name in output argument.
 */
inline constexpr void operator>> (protocol_t, const char *&name) noexcept
{
  name = "STUN";
}


/**
 * STUN protocol instance.
 */
static __turner_inline_var constexpr const protocol_t protocol{};


/**
 * STUN Binding message type (RFC5389, 18.1)
 */
using binding_t = protocol_t::message_type_t<0x001>;


/**
 * Return STUN Binding message \a name in output argument.
 */
inline constexpr void operator>> (binding_t, const char *&name) noexcept
{
  name = "binding";
}


/**
 * Instance of STUN Binding message type.
 */
static __turner_inline_var constexpr const binding_t binding{};


/**
 * STUN Binding success response message type
 */
using binding_success_t = binding_t::success_response_t;


/**
 * Instance of STUN Binding success resposen message type.
 */
static __turner_inline_var constexpr const binding_success_t binding_success;


} // namespace stun

__turner_end
