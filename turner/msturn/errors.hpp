#pragma once

/**
 * \file turner/msturn/errors.hpp
 * MS-TURN protocol errors.
 */


#include <turner/config.hpp>
#include <turner/error.hpp>


__turner_begin

namespace msturn {


/**
 * \defgroup MSTURN_errors MS-TURN errors
 * \{
 */


/**
 * Client should contact an alternate server for this request.
 */
__turner_inline_var constexpr const error_t try_alternate{
  300, "Try Alternate"
};


/**
 * Request does not include MESSAGE-INTEGRITY attribute.
 */
__turner_inline_var constexpr const error_t unauthorized{
  401, "Unauthorized"
};


/**
 * Request contained unknown attributes.
 */
__turner_inline_var constexpr const error_t unknown_attributes{
  420, "Unknown Attributes"
};


/**
 * Authentication failed.
 */
__turner_inline_var constexpr const error_t integrity_check_failure{
  431, "Integrity Check Failure"
};


/**
 * Request does not contain USERNAME attribute.
 */
__turner_inline_var constexpr const error_t missing_username{
  432, "Missing Username"
};


/**
 * Request does not contain REALM attribute.
 */
__turner_inline_var constexpr const error_t missing_realm{
  434, "Missing Realm"
};


/**
 * Request does not contain NONCE attribute.
 */
__turner_inline_var constexpr const error_t missing_nonce{
  435, "Missing Nonce"
};


/**
 * Request contained USERNAME attribute but it's value was not understood by
 * server.
 */
__turner_inline_var constexpr const error_t unknown_user{
  436, "Unknown User"
};


/**
 * Request contains NONCE attribute but the value was not valid.
 */
__turner_inline_var constexpr const error_t stale_nonce{
  438, "Stale Nonce"
};


/**
 * Allocation of transport addres failed for internal reasons.
 */
__turner_inline_var constexpr const error_t server_error{
  500, "Server Error"
};


/// \}


} // namespace msturn

__turner_end
