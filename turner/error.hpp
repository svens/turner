#pragma once // -*- C++ -*-

/**
 * \file turner/error.hpp
 * Turner library error codes
 */

#include <turner/__bits/lib.hpp>
#include <system_error>


__turner_begin


#define __turner_errc(Impl) \
	Impl(__0, "internal placeholder for not an error") \
	Impl(temporary_error, "temporary error")


/**
 * Turner error codes
 */
enum class errc: int
{
	#define __turner_errc_list(code, message) code,
	__turner_errc(__turner_errc_list)
	#undef __turner_errc_list
};


/**
 * Return Turner error category. The name() virtual function returns "turner".
 */
const std::error_category &error_category () noexcept;


/**
 * Make std::error_code from error code \a ec
 */
inline std::error_code make_error_code (errc ec) noexcept
{
	return std::error_code(static_cast<int>(ec), error_category());
}


__turner_end


namespace std {

template <>
struct is_error_code_enum<turner::errc>
	: true_type
{ };

} // namespace std
