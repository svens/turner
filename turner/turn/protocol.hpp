#pragma once

/**
 * \file turner/turn/protocol.hpp
 * TURN protocol (https://tools.ietf.org/html/rfc5766)
 */


#include <turner/config.hpp>
#include <turner/stun/protocol.hpp>


__turner_begin


namespace turn {


// Pull STUN long term credentials key calculator into TURN namespace.
using stun::make_integrity_calculator;


/**
 * TURN protocol message layout traits. As TURN is extension of STUN, it is
 * inherited from it's layout.
 */
struct protocol_traits_t
  : public stun::protocol_traits_t
{};


/**
 * Structure type describing TURN protocol message layout.
 */
using protocol_t = turner::protocol_t<protocol_traits_t>;


/**
 * TURN protocol instance.
 */
inline constexpr const protocol_t protocol{};


/**
 * Return TURN protocol \a name in output argument.
 */
inline constexpr void operator>> (protocol_t, const char *&name) noexcept
{
  name = "TURN";
}


} // namespace turn


__turner_end
