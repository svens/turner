#pragma once

/**
 * \file turner/stun/messages.hpp
 * STUN protocol messages.
 */


#include <turner/config.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <turner/stun/protocol.hpp>


__turner_begin

namespace stun {


/**
 * \defgroup STUN_messages STUN messages
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5389#section-18.1
 */

/**
 * STUN Binding message type.
 */
using binding_t = protocol_t::message_type_t<0x001>;


/**
 * Instance of STUN Binding message type.
 */
inline constexpr const binding_t binding{};


/**
 * STUN Binding success response message type
 */
using binding_success_t = binding_t::success_response_t;


/**
 * Instance of STUN Binding success resposen message type.
 */
inline constexpr const binding_success_t binding_success;


// \}


} // namespace stun

__turner_end
