#pragma once

/**
 * \file turner/codec/error.hpp
 * Codec errors
 */

#include <pal/result.hpp>
#include <system_error>

namespace turner
{

#define __turner_errc(Impl) \
	Impl(__0, "internal placeholder for not an error") \
	Impl(incomplete, "incomplete message") \
	Impl(invalid_magic_cookie, "invalid magic cookie") \
	Impl(unexpected_message_type, "unexpected message type") \
	Impl(unexpected_message_length, "unexpected message length") \
	Impl(fingerprint_mismatch, "fingerprint mismatch") \
	Impl(fingerprint_not_last, "fingerprint not last") \
	Impl(attribute_not_found, "attribute not found") \
	Impl(unexpected_attribute_length, "unexpected attribute length") \
	Impl(unexpected_attribute_value, "unexpected attribute value") \
	Impl(integrity_missing, "integrity missing") \
	Impl(integrity_mismatch, "integrity mismatch") \
	Impl(integrity_not_last, "integrity not last") \
	Impl(buffer_too_small, "buffer too small")

/// Codec errors
enum class errc: int
{
#define __turner_errc_enum(Code, Message) Code,
	__turner_errc(__turner_errc_enum)
#undef __turner_errc_enum
};

/// Return codec error category. The \c name() virtual function returns "turner".
const std::error_category &error_category () noexcept;

/// Make \c std::error_code from error code \a ec
inline std::error_code make_error_code (errc ec) noexcept
{
	return {static_cast<int>(ec), error_category()};
}

/// Make \c pal::unexpected from error code \a ec
inline pal::unexpected make_unexpected (errc ec) noexcept
{
	return pal::unexpected{make_error_code(ec)};
}

} // namespace turner

namespace std
{

template <>
struct is_error_code_enum<turner::errc>: true_type
{
};

} // namespace std
