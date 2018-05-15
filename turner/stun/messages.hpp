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
 * Binding message type.
 */
constexpr protocol_t::message_type_t<0x0001> binding;


/**
 * Binding success message type.
 */
constexpr protocol_t::message_type_t<0x0101> binding_success;


// \}


} // namespace stun

__turner_end
