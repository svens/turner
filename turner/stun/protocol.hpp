#pragma once

/**
 * \file turner/stun/protocol.hpp
 * STUN protocol (https://tools.ietf.org/html/rfc5389)
 */


#include <turner/config.hpp>
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


  /**
   * Attributes (TLV) padding boundary.
   */
  static __turner_inline_var constexpr const size_t padding_size = 4;
};


/**
 * Structure type describing STUN protocol message layout.
 */
using protocol_t = turner::protocol_t<protocol_traits_t>;


/**
 * STUN protocol instance.
 */
__turner_inline_var constexpr const protocol_t protocol;


/**
 * Return STUN protocol \a name in output argument.
 */
inline constexpr void operator>> (protocol_t, const char *&name) noexcept
{
  name = "STUN";
}


} // namespace stun

__turner_end
