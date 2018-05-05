#pragma once

/**
 * \file turner/turn/errors.hpp
 * TURN protocol errors.
 */


#include <turner/config.hpp>
#include <turner/turn/protocol.hpp>
#include <turner/stun/errors.hpp>


__turner_begin


namespace turn {


// Pull all STUN errors into TURN namespace
using stun::try_alternate;
using stun::bad_request;
using stun::unauthorized;
using stun::unknown_attribute;
using stun::stale_nonce;
using stun::server_error;


/**
 * \defgroup TURN_errors TURN errors
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5766#section-15
 * \see https://tools.ietf.org/html/rfc6156#section-10.2
 */


/**
 * The request was valid but cannot be performed due to administrative or
 * similar restriction.
 */
inline constexpr const error_t forbidden{
  403, "Forbidden"
};


/**
 * Request requires an allocation but no allocation exists, or a request
 * requires no allocation but an allocation exists.
 */
inline constexpr const error_t allocation_mismatch{
  437, "Allocation Mismatch"
};


/**
 * The server does not support the address family requested by the client.
 */
inline constexpr const error_t unsupported_address_family{
  440, "Address Family not Supported"
};


/**
 * The credentials in request do not match those used to create the
 * allocation.
 */
inline constexpr const error_t wrong_credentials{
  441, "Wrong Credentials"
};


/**
 * The Allocate request asked for transport protocol between server and the
 * peer that the server does not support.
 */
inline constexpr const error_t unsupported_transport_protocol{
  442, "Unsupported Transport Protocol"
};


/**
 * A peer address was of a different address family than that of the relayed
 * transport address of the allocation.
 *
 */
inline constexpr const error_t peer_address_family_mismatch{
  443, "Peer Address Family Mismatch"
};


/**
 * No more allocations using this username can be created at the present time.
 */
inline constexpr const error_t allocation_quota_reached{
  486, "Allocation Quota Reached"
};


/**
 * The server is unable to carry out the request dues to some capacity limit
 * being reached.
 */
inline constexpr const error_t insufficient_capacity{
  508, "Insufficient Capacity"
};


/// \}


} // namespace turn


__turner_end
