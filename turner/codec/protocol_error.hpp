#pragma once

/**
 * \file turner/codec/protocol_error.hpp
 * Protocol error codes
 */

#include <cstdint>
#include <string_view>
#include <system_error>

namespace turner
{

#define __turner_protocol_errc(Impl) \
	Impl(300, try_alternate, "Try Alternate") \
	Impl(400, bad_request, "Bad Request") \
	Impl(401, unauthorized, "Unauthorized") \
	Impl(403, forbidden, "Forbidden") \
	Impl(420, unknown_attribute, "Unknown Attribute") \
	Impl(431, integrity_check_failure, "Integrity Check Failure") \
	Impl(432, missing_username, "Missing Username") \
	Impl(434, missing_realm, "Missing Realm") \
	Impl(435, missing_nonce, "Missing Nonce") \
	Impl(436, unknown_user, "Unknown User") \
	Impl(437, allocation_mismatch, "Allocation Mismatch") \
	Impl(438, stale_nonce, "Stale Nonce") \
	Impl(440, address_family_not_supported, "Address Family not Supported") \
	Impl(441, wrong_credentials, "Wrong Credentials") \
	Impl(442, unsupported_transport_protocol, "Unsupported Transport Protocol") \
	Impl(443, peer_address_family_mismatch, "Peer Address Family Mismatch") \
	Impl(486, allocation_quota_reached, "Allocation Quota Reached") \
	Impl(500, server_error, "Server Error") \
	Impl(508, insufficient_capacity, "Insufficient Capacity")

/// Error codes carried in protocol error responses
enum class protocol_errc: uint16_t
{
#define __turner_protocol_errc_enum(Value, Code, Message) Code = (Value),
	__turner_protocol_errc(__turner_protocol_errc_enum)
#undef __turner_protocol_errc_enum
};

/// Return reason phrase of \a code, or empty if \a code is unknown
constexpr std::string_view reason_phrase (protocol_errc code) noexcept
{
	switch (code)
	{
#define __turner_protocol_errc_case(Value, Code, Message) case protocol_errc::Code: return Message;
		__turner_protocol_errc(__turner_protocol_errc_case)
#undef __turner_protocol_errc_case
	}
	return {};
}

/// ERROR-CODE attribute native value. Reason refers to bytes in message when read.
struct protocol_error
{
	/// Error code
	protocol_errc code;

	/// Reason phrase
	std::string_view reason;

	/// Construct with \a error and its reason_phrase()
	constexpr protocol_error (protocol_errc error) noexcept
		: code{error}
		, reason{reason_phrase(error)}
	{
	}

	/// Construct with \a error and \a phrase
	constexpr protocol_error (protocol_errc error, std::string_view phrase) noexcept
		: code{error}
		, reason{phrase}
	{
	}

	/// Return true if \a a has code \a b, regardless of reason
	friend constexpr bool operator== (const protocol_error &a, protocol_errc b) noexcept
	{
		return a.code == b;
	}
};

/// Return protocol error category. The \c name() virtual function returns "turner::protocol".
const std::error_category &protocol_error_category () noexcept;

/// Make \c std::error_code from error code \a ec
inline std::error_code make_error_code (protocol_errc ec) noexcept
{
	return {static_cast<int>(ec), protocol_error_category()};
}

} // namespace turner

namespace std
{

template <>
struct is_error_code_enum<turner::protocol_errc>: true_type
{
};

} // namespace std
