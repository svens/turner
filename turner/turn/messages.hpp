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
 * Allocation message type.
 */
inline constexpr const protocol_t::message_type_t<0x0003> allocation;


/**
 * Allocation success message type.
 */
inline constexpr const protocol_t::message_type_t<0x0103> allocation_success;


/**
 * Allocation error message type.
 */
inline constexpr const protocol_t::message_type_t<0x0113> allocation_error;


/**
 * Refresh message type.
 */
inline constexpr const protocol_t::message_type_t<0x0004> refresh;


/**
 * Refresh success message type.
 */
inline constexpr const protocol_t::message_type_t<0x0104> refresh_success;


/**
 * Refresh error message type.
 */
inline constexpr const protocol_t::message_type_t<0x0114> refresh_error;


/**
 * Send Indication message type.
 */
inline constexpr const protocol_t::message_type_t<0x0016> send_indication;


/**
 * Data Indication message type.
 */
inline constexpr const protocol_t::message_type_t<0x0017> data_indication;


/**
 * Create Permission message type.
 */
inline constexpr const protocol_t::message_type_t<0x0008> create_permission;


/**
 * Create Permission success message type.
 */
inline constexpr const protocol_t::message_type_t<0x0108> create_permission_success;


/**
 * Create Permission error message type.
 */
inline constexpr const protocol_t::message_type_t<0x0118> create_permission_error;


/**
 * Channel Bind message type.
 */
inline constexpr const protocol_t::message_type_t<0x0009> channel_bind;


/**
 * Channel Bind success message type.
 */
inline constexpr const protocol_t::message_type_t<0x0109> channel_bind_success;


/**
 * Channel Bind error message type.
 */
inline constexpr const protocol_t::message_type_t<0x0119> channel_bind_error;


// \}


} // namespace turn


__turner_end
