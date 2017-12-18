#pragma once

/**
 * \file turner/turn/messages.hpp
 * TURN protocol messages.
 */


#include <turner/config.hpp>
#include <turner/turn/protocol.hpp>
#include <turner/stun/messages.hpp>


__turner_begin


namespace turn {


/**
 * \defgroup TURN_messages TURN messages
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5766#section-13
 */


/**
 * TURN Allocation message type.
 */
using allocation_t = protocol_t::message_type_t<0x003>;


/**
 * Instance of TURN Allocation message type.
 */
inline constexpr const allocation_t allocation;


/**
 * Return TURN Allocation message \a name in output argument.
 */
inline constexpr void operator>> (allocation_t, const char *&name) noexcept
{
  name = "allocation";
}


/**
 * TURN Allocation success response message type
 */
using allocation_success_t = allocation_t::success_response_t;


/**
 * Instance of TURN Allocation success resposen message type.
 */
inline constexpr const allocation_success_t allocation_success;


/**
 * Return TURN Allocation success response message \a name in output argument.
 */
inline constexpr void operator>> (allocation_success_t, const char *&name) noexcept
{
  name = "allocation_success";
}


/**
 * TURN Allocation error response message type
 */
using allocation_error_t = allocation_t::error_response_t;


/**
 * Instance of TURN Allocation error resposen message type.
 */
inline constexpr const allocation_error_t allocation_error;


/**
 * Return TURN Allocation error response message \a name in output argument.
 */
inline constexpr void operator>> (allocation_error_t, const char *&name) noexcept
{
  name = "allocation_error";
}


/**
 * TURN Refresh message type.
 */
using refresh_t = protocol_t::message_type_t<0x004>;


/**
 * Instance of TURN Refresh message type.
 */
inline constexpr const refresh_t refresh;


/**
 * Return TURN Refresh message \a name in output argument.
 */
inline constexpr void operator>> (refresh_t, const char *&name) noexcept
{
  name = "refresh";
}


/**
 * TURN Refresh success response message type
 */
using refresh_success_t = refresh_t::success_response_t;


/**
 * Instance of TURN Refresh success resposen message type.
 */
inline constexpr const refresh_success_t refresh_success;


/**
 * Return TURN Refresh success response message \a name in output argument.
 */
inline constexpr void operator>> (refresh_success_t, const char *&name) noexcept
{
  name = "refresh_success";
}


/**
 * TURN Refresh error response message type
 */
using refresh_error_t = refresh_t::error_response_t;


/**
 * Instance of TURN Refresh error resposen message type.
 */
inline constexpr const refresh_error_t refresh_error;


/**
 * Return TURN Refresh error response message \a name in output argument.
 */
inline constexpr void operator>> (refresh_error_t, const char *&name) noexcept
{
  name = "refresh_error";
}


/**
 * TURN Send indication message type.
 */
using send_indication_t = protocol_t::message_type_t<0x006>::indication_t;


/**
 * Instance of TURN Send indication message type.
 */
inline constexpr const send_indication_t send_indication;


/**
 * Return TURN Send indication message \a name in output argument.
 */
inline constexpr void operator>> (send_indication_t, const char *&name) noexcept
{
  name = "send_indication";
}


/**
 * TURN Data indication message type.
 */
using data_indication_t = protocol_t::message_type_t<0x007>::indication_t;


/**
 * Instance of TURN Data indication message type.
 */
inline constexpr const data_indication_t data_indication;


/**
 * Return TURN Data indication message \a name in output argument.
 */
inline constexpr void operator>> (data_indication_t, const char *&name) noexcept
{
  name = "data_indication";
}


/**
 * TURN CreatePermission message type.
 */
using create_permission_t = protocol_t::message_type_t<0x008>;


/**
 * Instance of TURN CreatePermission message type.
 */
inline constexpr const create_permission_t create_permission;


/**
 * Return TURN CreatePermission message \a name in output argument.
 */
inline constexpr void operator>> (create_permission_t, const char *&name) noexcept
{
  name = "create_permission";
}


/**
 * TURN CreatePermission success response message type
 */
using create_permission_success_t = create_permission_t::success_response_t;


/**
 * Instance of TURN CreatePermission success resposen message type.
 */
inline constexpr const create_permission_success_t create_permission_success;


/**
 * Return TURN CreatePermission success response message \a name in output argument.
 */
inline constexpr void operator>> (create_permission_success_t, const char *&name) noexcept
{
  name = "create_permission_success";
}


/**
 * TURN CreatePermission error response message type
 */
using create_permission_error_t = create_permission_t::error_response_t;


/**
 * Instance of TURN CreatePermission error resposen message type.
 */
inline constexpr const create_permission_error_t create_permission_error;


/**
 * Return TURN CreatePermission error response message \a name in output argument.
 */
inline constexpr void operator>> (create_permission_error_t, const char *&name) noexcept
{
  name = "create_permission_error";
}


/**
 * TURN ChannelBind message type.
 */
using channel_bind_t = protocol_t::message_type_t<0x009>;


/**
 * Instance of TURN ChannelBind message type.
 */
inline constexpr const channel_bind_t channel_bind;


/**
 * Return TURN ChannelBind message \a name in output argument.
 */
inline constexpr void operator>> (channel_bind_t, const char *&name) noexcept
{
  name = "channel_bind";
}


/**
 * TURN ChannelBind success response message type
 */
using channel_bind_success_t = channel_bind_t::success_response_t;


/**
 * Instance of TURN ChannelBind success resposen message type.
 */
inline constexpr const channel_bind_success_t channel_bind_success;


/**
 * Return TURN ChannelBind success response message \a name in output argument.
 */
inline constexpr void operator>> (channel_bind_success_t, const char *&name) noexcept
{
  name = "channel_bind_success";
}


/**
 * TURN ChannelBind error response message type
 */
using channel_bind_error_t = channel_bind_t::error_response_t;


/**
 * Instance of TURN ChannelBind error resposen message type.
 */
inline constexpr const channel_bind_error_t channel_bind_error;


/**
 * Return TURN ChannelBind error response message \a name in output argument.
 */
inline constexpr void operator>> (channel_bind_error_t, const char *&name) noexcept
{
  name = "channel_bind_error";
}


// \}


} // namespace turn


__turner_end
