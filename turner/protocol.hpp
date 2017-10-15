#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>
#include <sal/byte_order.hpp>
#include <type_traits>


__turner_begin


/**
 * Common base class for protocol classes.
 */
class protocol_base_t
{};


/**
 * Generalised protocol description class.
 *
 * Specific Protocol defines own trait and wrap it into basic_protocol_t which
 * provides common API for message parsing.
 */
template <typename Protocol>
class basic_protocol_t
  : public protocol_base_t
{
public:

  /**
   */
  using cookie_t = std::remove_reference_t<decltype(Protocol::cookie())>;


  /**
   */
  template <typename It>
  static constexpr any_message_t<Protocol> from_wire (It first, It last,
    std::error_code &error) noexcept
  {
    return from_wire(__bits::to_ptr(first), __bits::to_ptr(last), error);
  }


  /**
   */
  template <typename It>
  static constexpr any_message_t<Protocol> from_wire (It first, It last)
  {
    return from_wire(__bits::to_ptr(first), __bits::to_ptr(last),
      sal::throw_on_error("basic_protocol_t::from_wire")
    );
  }


  /**
   * Write to \a stream \a Protocol name. If name is not defined, nothing is
   * written.
   */
  friend std::ostream &operator<< (std::ostream &stream, basic_protocol_t)
  {
    if constexpr (__bits::has_name_v<Protocol>)
    {
      stream << Protocol::name();
    }
    return stream;
  }


private:

  static constexpr any_message_t<Protocol> from_wire (
    const uint8_t *first,
    const uint8_t *last,
    std::error_code &error
  ) noexcept;
};


template <typename Protocol>
constexpr any_message_t<Protocol> basic_protocol_t<Protocol>::from_wire (
  const uint8_t *first,
  const uint8_t *last,
  std::error_code &error) noexcept
{
  // validate arguments
  if (!first || !last || first > last)
  {
    error = std::make_error_code(std::errc::invalid_argument);
    return {};
  }
  if (last - first < Protocol::header_size())
  {
    error = make_error_code(errc::insufficient_data);
    return {};
  }

  // message type (2 highest bits must be 00, RFC5389, section 6)
  any_message_t<Protocol> message(first, last);
  if ((message.type() & 0b1100'0000'0000'0000) != 0)
  {
    error = make_error_code(errc::invalid_message_type);
    return {};
  }

  // message length (must be padded to 4B boundary)
  if (message.length() % 4 != 0)
  {
    error = make_error_code(errc::invalid_message_length);
    return {};
  }

  // cookie
  if (message.cookie() != Protocol::cookie())
  {
    error = make_error_code(errc::invalid_message_cookie);
    return {};
  }

  error.clear();
  return message;
}


__turner_end
