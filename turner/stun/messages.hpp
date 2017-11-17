#pragma once

/**
 * \file turner/stun/methods.hpp
 * STUN protocol methods.
 */


#include <turner/config.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <turner/stun/protocol.hpp>


__turner_begin

namespace stun {


/**
 * \defgroup STUN_methods STUN methods
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
__turner_inline_var constexpr const binding_t binding{};

/**
 * STUN Binding success response message type
 */
using binding_success_t = binding_t::success_response_t;

/**
 * Instance of STUN Binding success resposen message type.
 */
__turner_inline_var constexpr const binding_success_t binding_success;

// \}


/**
 * Return STUN Binding message \a name in output argument.
 */
inline constexpr void operator>> (binding_t, const char *&name) noexcept
{
  name = "binding";
}


/**
 * Return STUN Binding success response message \a name in output argument.
 */
inline constexpr void operator>> (binding_success_t, const char *&name) noexcept
{
  name = "binding_success";
}


} // namespace stun

__turner_end
