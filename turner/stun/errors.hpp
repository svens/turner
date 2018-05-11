#pragma once

/**
 * \file turner/stun/errors.hpp
 * STUN protocol errors.
 */


#include <turner/config.hpp>
#include <turner/error.hpp>


__turner_begin

namespace stun {


/**
 * \defgroup STUN_errors STUN errors
 * \{
 *
 * \see https://tools.ietf.org/html/rfc5389#section-15.6
 */


/**
 * Client should contact an alternate server for this request.
 */
constexpr error_t try_alternate{
  300, "Try Alternate"
};


/**
 * The request was malformed.
 */
constexpr error_t bad_request{
  400, "Bad Request"
};


/**
 * The request did not contain the correct credentials to proceed.
 */
constexpr error_t unauthorized{
  401, "Unauthorized"
};


/**
 * The server received a STUN packet containing comprehension-required
 * attribute that it did not understand.
 */
constexpr error_t unknown_attribute{
  420, "Unknown Attribute"
};


/**
 * The NONCE used by the client was no longer valid.
 */
constexpr error_t stale_nonce{
  438, "Stale Nonce"
};


/**
 * The server has suffered a temporary error. The client should try again.
 */
constexpr error_t server_error{
  500, "Server Error"
};


/// \}


} // namespace stun

__turner_end
