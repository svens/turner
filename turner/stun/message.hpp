#pragma once

/**
 * \file turner/stun/message.hpp
 */


#include <turner/config.hpp>
#include <turner/message.hpp>
#include <array>


__turner_begin


namespace stun {


/**
 * STUN Binding request
 */
__turner_inline_var constexpr message_type_t
  binding = request_v<0x001>;

/**
 * STUN Binding success
 */
__turner_inline_var constexpr message_type_t
  binding_success = success_response_v<binding>;


/**
 */
struct protocol_t
{
  /**
   */
  static constexpr bool is_valid_message_type (message_type_t type) noexcept
  {
    return type == binding
      || type == binding_success;
  }

  /**
   */
  static __turner_inline_var constexpr size_t header_size = 20;

  /**
   */
  static __turner_inline_var constexpr std::array<uint8_t, 4> cookie =
  {
    { 0x21, 0x12, 0xa4, 0x42 }
  };
  static __turner_inline_var constexpr size_t cookie_offset = 4;

  /**
   */
  static __turner_inline_var constexpr size_t transaction_id_size = 12;
  static __turner_inline_var constexpr size_t transaction_id_offset = 8;
};


/**
 */
using message_t = basic_message_t<protocol_t>;


/**
 */
template <typename Data>
inline message_t message (const Data &data, std::error_code &error) noexcept
{
  return message_t::make(data.begin(), data.end(), error);
}


/**
 */
template <typename Data>
inline message_t message (const Data &data)
{
  return message(data, sal::throw_on_error("stun::message"));
}


} // namespace stun


__turner_end
