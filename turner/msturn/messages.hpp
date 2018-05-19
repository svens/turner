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
 * Allocate message type.
 */
constexpr protocol_t::message_type_t<0x0003> allocate;


/**
 * Allocate success message type.
 */
constexpr protocol_t::message_type_t<0x0103> allocate_success;


/**
 * Allocate error message type.
 */
constexpr protocol_t::message_type_t<0x0113> allocate_error;


/**
 * Send message type.
 */
constexpr protocol_t::message_type_t<0x0004> send;


/**
 * Data Indication message type.
 */
constexpr protocol_t::message_type_t<0x0115> data_indication;


/**
 * Set Active Destination message type.
 */
constexpr protocol_t::message_type_t<0x0006> set_active_destination;


/**
 * Set Active Destination success message type.
 */
constexpr protocol_t::message_type_t<0x0106> set_active_destination_success;


/**
 * Set Active Destination error message type.
 */
constexpr protocol_t::message_type_t<0x0116> set_active_destination_error;


// \}


} // namespace msturn

__turner_end
