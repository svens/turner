#pragma once

/**
 * \file turner/stun/message.hpp
 */


#include <turner/config.hpp>
#include <turner/message.hpp>


__turner_begin


namespace stun {


/**
 * STUN Binding request
 */
inline constexpr message_type_t binding = method_v<0x001>;

/**
 * STUN Binding success
 */
inline constexpr message_type_t binding_success = success_response_v<binding>;


/**
 */
struct protocol_t
{
  /**
   */
  static inline constexpr size_t transaction_id_size = 12;

  /**
   */
  static inline constexpr size_t transaction_id_offset = 4;


  /**
   */
  static basic_message_t<protocol_t> message (
    const uint8_t *first,
    const uint8_t *last,
    std::error_code &error
  ) noexcept;
};


/**
 */
using message_t = basic_message_t<protocol_t>;


/**
 */
template <typename Data>
inline message_t message (const Data &data, std::error_code &error) noexcept
{
  return protocol_t::message(data.data(), data.data() + data.size(), error);
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
