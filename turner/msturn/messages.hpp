#pragma once

/**
 * \file turner/msturn/messages.hpp
 * MS-TURN protocol messages.
 */


#include <turner/config.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <turner/msturn/protocol.hpp>


__turner_begin

namespace msturn {


/**
 * \defgroup MSTURN_messages MS-TURN messages
 * \{
 *
 * \see https://msdn.microsoft.com/en-us/library/dd946797(v=office.12).aspx
 */


/**
 * MS-TURN Allocate message type.
 */
using allocate_t = protocol_t::message_type_t<0x003>;


/**
 * Instance of MS-TURN Allocate message type.
 */
__turner_inline_var constexpr const allocate_t allocate;


/**
 * Return MS-TURN Allocate message \a name in output argument.
 */
inline constexpr void operator>> (allocate_t, const char *&name) noexcept
{
  name = "allocate";
}


/**
 * MS-TURN Allocate success response message type
 */
using allocate_success_t = allocate_t::success_response_t;


/**
 * Instance of MS-TURN Allocate success resposen message type.
 */
__turner_inline_var constexpr const allocate_success_t allocate_success;


/**
 * Return MS-TURN Allocate success response message \a name in output argument.
 */
inline constexpr void operator>> (allocate_success_t, const char *&name) noexcept
{
  name = "allocate_success";
}


/**
 * MS-TURN Allocate error response message type
 */
using allocate_error_t = allocate_t::error_response_t;


/**
 * Instance of MS-TURN Allocate error resposen message type.
 */
__turner_inline_var constexpr const allocate_error_t allocate_error;


/**
 * Return MS-TURN Allocate error response message \a name in output argument.
 */
inline constexpr void operator>> (allocate_error_t, const char *&name) noexcept
{
  name = "allocate_error";
}


/**
 * MS-TURN Set Active Destination message type.
 */
using set_active_destination_t = protocol_t::message_type_t<0x006>;


/**
 * Instance of MS-TURN Set Active Destination message type.
 */
__turner_inline_var constexpr const set_active_destination_t set_active_destination;


/**
 * Return MS-TURN Set Active Destination message \a name in output argument.
 */
inline constexpr void operator>> (set_active_destination_t, const char *&name) noexcept
{
  name = "set_active_destination";
}


/**
 * MS-TURN Set Active Destination success response message type
 */
using set_active_destination_success_t = set_active_destination_t::success_response_t;


/**
 * Instance of MS-TURN Set Active Destination success resposen message type.
 */
__turner_inline_var constexpr const set_active_destination_success_t set_active_destination_success;


/**
 * Return MS-TURN Set Active Destination success response message \a name in output argument.
 */
inline constexpr void operator>> (set_active_destination_success_t, const char *&name) noexcept
{
  name = "set_active_destination_success";
}


/**
 * MS-TURN Set Active Destination error response message type
 */
using set_active_destination_error_t = set_active_destination_t::error_response_t;


/**
 * Instance of MS-TURN Set Active Destination error resposen message type.
 */
__turner_inline_var constexpr const set_active_destination_error_t set_active_destination_error;


/**
 * Return MS-TURN Set Active Destination error response message \a name in output argument.
 */
inline constexpr void operator>> (set_active_destination_error_t, const char *&name) noexcept
{
  name = "set_active_destination_error";
}


/**
 * MS-TURN Send message type.
 */
using send_t = protocol_t::message_type_t<0x004>;


/**
 * Instance of MS-TURN Send message type.
 */
__turner_inline_var constexpr const send_t send;


/**
 * Return MS-TURN Send message \a name in output argument.
 */
inline constexpr void operator>> (send_t, const char *&name) noexcept
{
  name = "send";
}


/**
 * MS-TURN Data Indication message type.
 *
 * \note For some reason, Data Indicaton encoding is not usual STUN bit-coded
 * message type: here indication is encoded as error response.
 */
using data_indication_t = protocol_t::message_type_t<0x005>::indication_t;


/**
 * Instance of MS-TURN Data Indication message type.
 */
__turner_inline_var constexpr const data_indication_t data_indication;


/**
 * Return MS-TURN Data Indication message \a name in output argument.
 */
inline constexpr void operator>> (data_indication_t, const char *&name) noexcept
{
  name = "data_indication";
}


// \}


} // namespace msturn

__turner_end
