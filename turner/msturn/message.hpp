#pragma once

/**
 * \file turner/msturn/message.hpp
 */


#include <turner/config.hpp>
#include <turner/message.hpp>
#include <array>


__turner_begin


namespace msturn {


/**
 * MS-TURN Allocate request
 */
__turner_inline_var constexpr message_type_t
  allocate = request_v<0x003>;

/**
 * MS-TURN Allocate success response
 */
__turner_inline_var constexpr message_type_t
  allocate_success = success_response_v<allocate>;

/**
 * MS-TURN Allocate error response
 */
__turner_inline_var constexpr message_type_t
  allocate_error = error_response_v<allocate>;


/**
 * MS-TURN Send
 */
__turner_inline_var constexpr message_type_t
  send = request_v<0x004>;


/**
 * MS-TURN Data Indication
 */
__turner_inline_var constexpr message_type_t
  data_indication = indication_v<0x005>;


/**
 * MS-TURN Set Active Destination request
 */
__turner_inline_var constexpr message_type_t
  set_active_destination = request_v<0x006>;

/**
 * MS-TURN Set Active Destination success response
 */
__turner_inline_var constexpr message_type_t
  set_active_destination_success = success_response_v<set_active_destination>;

/**
 * MS-TURN Set Active Destination error response
 */
__turner_inline_var constexpr message_type_t
  set_active_destination_error = error_response_v<set_active_destination>;


/**
 * Although MS-TURN says header size is 20B, it requires first attribute to be
 * Magic Cookie in all requests. Here, for uniform handling with STUN/TURN,
 * let's make it part of header i.e.:
 *  - header size 20B -> 28B
 *  - cookie offset 24 -> 20
 *  - cookie content contains whole TLV
 */
struct protocol_t
{
  /**
   */
  static constexpr bool is_valid_message_type (message_type_t type) noexcept
  {
    return type == allocate
      || type == allocate_success
      || type == allocate_error
      || type == send
      || type == data_indication
      || type == set_active_destination
      || type == set_active_destination_success
      || type == set_active_destination_error;
  }

  /**
   */
  static __turner_inline_var constexpr size_t header_size = 28;

  /**
   */
  static __turner_inline_var constexpr std::array<uint8_t, 8> cookie =
  {{
     0x00, 0x0f,                // Type
     0x00, 0x04,                // Length
     0x72, 0xc6, 0x4b, 0xc6,    // Magic Cookie
  }};
  static __turner_inline_var constexpr size_t cookie_offset = 20;

  /**
   */
  static __turner_inline_var constexpr size_t transaction_id_size = 16;
  static __turner_inline_var constexpr size_t transaction_id_offset = 4;
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
  return message(data, sal::throw_on_error("msturn::message"));
}


} // namespace msturn


__turner_end
