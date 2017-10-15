#pragma once

/**
 * \file turner/error.hpp
 * Turner library error codes
 */

#include <turner/config.hpp>
#include <sal/error.hpp>
#include <system_error>


__turner_begin


/**
 * Turner library error codes
 */
enum class errc
{
  insufficient_data = 1,
  invalid_message_type,
  invalid_message_length,
  invalid_message_cookie,
};


/**
 * Return reference to Turner library error category. The method name() return
 * pointer to string "turner".
 */
const std::error_category &category () noexcept;


/**
 * Make std::error_code from errc \a e
 */
inline std::error_code make_error_code (errc e) noexcept
{
  return std::error_code(static_cast<int>(e), category());
}


/**
 * Make std::error_condition from errc \a e
 */
inline std::error_condition make_error_condition (errc e) noexcept
{
  return std::error_condition(static_cast<int>(e), category());
}


__turner_end


namespace std {

template <>
struct is_error_condition_enum<turner::errc>
  : public true_type
{};

} // namespace std
