#pragma once

/**
 * \file turner/error.hpp
 * Turner library error codes
 */

#include <turner/config.hpp>
#include <sal/error.hpp>
#include <ostream>
#include <string_view>
#include <system_error>


__turner_begin


/**
 * Turner library error codes
 */
enum class errc
{
  insufficient_header_data = 1,
  insufficient_payload_data,
  invalid_message_type,
  invalid_message_length,
  invalid_message_cookie,
  unexpected_message_type,
  unknown_comprehension_required,
  attribute_not_found,
  unexpected_attribute_length,
  unexpected_attribute_value,
  not_enough_room,
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


/**
 * Protocol error code and corresponding message.
 */
struct error_t
{
  /**
   * Error code (semantic is defined by specific protocol)
   */
  const uint16_t code{};

  /**
   * Error phase (recommended content is defined by specific protocol)
   */
  const std::string_view message{};


  constexpr error_t () = default;


  /**
   * Create new error instance.
   */
  constexpr error_t (uint16_t code, const std::string_view &message) noexcept
    : code(code)
    , message(message)
  {}


  /**
   * Create new error instance.
   */
  template <size_t N>
  constexpr error_t (uint16_t code, const char (&message)[N]) noexcept
    : code(code)
    , message(message, N - 1)
  {}


  /**
   * Return true if \a this error code is same as \a that.
   */
  bool operator== (const error_t &that) const noexcept
  {
    return code == that.code;
  }


  /**
   * Return true if \a this error code is differnt that \a that.
   */
  bool operator!= (const error_t &that) const noexcept
  {
    return code != that.code;
  }
};


/**
 * Success code
 * \note This is not part of STUN/TURN/MSTURN protocols. Used only internally
 */
inline constexpr const error_t success{0, "Success"};


/**
 * Print into \a stream error code in format 'code message'.
 */
inline std::ostream &operator<< (std::ostream &stream, const error_t &error)
{
  return (stream << error.code << ' ' << error.message);
}


__turner_end


namespace std {

template <>
struct is_error_condition_enum<turner::errc>
  : public true_type
{};

} // namespace std
