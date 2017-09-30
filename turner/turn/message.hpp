#pragma once

/**
 * \file turner/turn/message.hpp
 */


#include <turner/config.hpp>
#include <turner/message.hpp>
#include <turner/stun/message.hpp>
#include <array>


__turner_begin


namespace turn {


/**
 * TURN Allocation request
 */
__turner_inline_var constexpr message_type_t allocation = request_v<0x003>;

/**
 * TURN Allocation success response
 */
__turner_inline_var constexpr message_type_t allocation_success = success_response_v<allocation>;

/**
 * TURN Allocation error response
 */
__turner_inline_var constexpr message_type_t allocation_error = error_response_v<allocation>;


/**
 * TURN Refresh request
 */
__turner_inline_var constexpr message_type_t refresh = request_v<0x004>;

/**
 * TURN Refresh success response
 */
__turner_inline_var constexpr message_type_t refresh_success = success_response_v<refresh>;

/**
 * TURN Refresh error response
 */
__turner_inline_var constexpr message_type_t refresh_error = error_response_v<refresh>;


/**
 * TURN Send
 */
__turner_inline_var constexpr message_type_t send = request_v<0x006>;

/**
 * TURN Send Indication
 */
__turner_inline_var constexpr message_type_t send_indication = indication_v<send>;


/**
 * TURN Data
 */
__turner_inline_var constexpr message_type_t data = request_v<0x007>;

/**
 * TURN Data Indication
 */
__turner_inline_var constexpr message_type_t data_indication = indication_v<data>;


/**
 * TURN CreatePermission request
 */
__turner_inline_var constexpr message_type_t create_permission = request_v<0x008>;

/**
 * TURN CreatePermission success response
 */
__turner_inline_var constexpr message_type_t create_permission_success = success_response_v<create_permission>;

/**
 * TURN CreatePermission error response
 */
__turner_inline_var constexpr message_type_t create_permission_error = error_response_v<create_permission>;


/**
 * TURN ChannelBind request
 */
__turner_inline_var constexpr message_type_t channel_bind = request_v<0x009>;

/**
 * TURN ChannelBind success response
 */
__turner_inline_var constexpr message_type_t channel_bind_success = success_response_v<channel_bind>;

/**
 * TURN ChannelBind error response
 */
__turner_inline_var constexpr message_type_t channel_bind_error = error_response_v<channel_bind>;


/**
 */
struct protocol_t
  : public stun::protocol_t
{
  /**
   */
  static constexpr bool is_valid_message_type (message_type_t type) noexcept
  {
    type &= ~__bits::class_mask;
    return stun::protocol_t::is_valid_message_type(type)
      || type == allocation
      || type == refresh
      || type == send
      || type == data
      || type == create_permission
      || type == channel_bind
    ;
  }
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
  return message(data, sal::throw_on_error("turn::message"));
}


} // namespace turn


__turner_end
