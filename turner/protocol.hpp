#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>
#include <array>
#include <ostream>
#include <type_traits>


__turner_begin


/**
 * Generalised protocol description class.
 *
 * Specific Protocol defines own trait and wrap it into basic_protocol_t which
 * provides common API for message parsing.
 */
template <typename Protocol>
class basic_protocol_t
{
public:

  /**
   * Protocol traits.
   */
  using traits_t = Protocol;

  /**
   * Cookie type for \a Protocol message.
   */
  using cookie_t = std::array<uint8_t, Protocol::cookie.max_size()>;

  /**
   * Transaction ID type for \a Protocol message.
   */
  using transaction_id_t = std::array<uint8_t, Protocol::transaction_id_size>;

  /**
   * Message \a Type as defined by \a Protocol
   */
  template <uint16_t Type>
  using message_type_t = basic_message_type_t<Protocol, Type>;


  /**
   * Return \a Protocol name (if defined).
   *
   * \a Protocol name is defined, if \c "void operator>> (Protocol, const char *&name)"
   * is provided. On operator invocation, name should be assigned to
   * \a Protocol name.
   *
   * If this operator is not provided, nullptr is returned.
   */
  static constexpr const char *name () noexcept
  {
    if constexpr (__bits::has_name_getter_v<basic_protocol_t>)
    {
      const char *result{};
      basic_protocol_t{} >> result;
      return result;
    }
    else
    {
      return nullptr;
    }
  }


  /**
   * Return instance of message \a Type as defined by \a Protocol.
   */
  template <uint16_t Type>
  static constexpr message_type_t<Type> message_type () noexcept
  {
    message_type_t<Type>::expect_request_class();
    return {};
  }


  /**
   * Return pointer to message instance wrapping raw network format in range
   * [\a first, \a last).
   *
   * Returned pointed object is not actually instantiated but overlayed onto
   * specified memory range [\a first, \a last). Each time any of
   * any_message_t method is invocated, it parses fields relative to \a this.
   *
   * If during call to from_wire() message validation fails, nullptr is
   * returned and error is set to code describing failure.
   */
  template <typename It>
  static constexpr const any_message_t<Protocol> *from_wire (It first, It last,
    std::error_code &error) noexcept
  {
    auto begin = __bits::to_ptr(first);
    return from_wire(begin, begin + (last - first), error);
  }


  /**
   * Return pointer to message instance wrapping raw network format in range
   * [\a first, \a last).
   *
   * Returned pointed object is not actually instantiated but overlayed onto
   * specified memory range [\a first, \a last). Each time any of
   * any_message_t method is invocated, it parses fields relative to \a this.
   *
   * \throws std::system_error if during call to from_wire() message
   * validation fails.
   */
  template <typename It>
  static constexpr const any_message_t<Protocol> *from_wire (It first, It last)
  {
    return from_wire(first, last,
      sal::throw_on_error("basic_protocol::from_wire")
    );
  }


  /**
   * Write to \a stream \a Protocol name. If name is not defined, nothing is
   * written.
   */
  friend std::ostream &operator<< (std::ostream &stream, basic_protocol_t)
  {
    if constexpr (name() != nullptr)
    {
      return (stream << name());
    }
    else
    {
      return stream;
    }
  }


private:

  static constexpr const any_message_t<Protocol> *from_wire (
    const uint8_t *first,
    const uint8_t *last,
    std::error_code &error
  ) noexcept;
};


template <typename Protocol>
constexpr const any_message_t<Protocol> *basic_protocol_t<Protocol>::from_wire (
  const uint8_t *first,
  const uint8_t *last,
  std::error_code &error) noexcept
{
  // validate arguments
  if (size_t(last - first) < Protocol::header_size)
  {
    error = make_error_code(errc::insufficient_header_data);
    return {};
  }

  // passed initial checks, overlay any_message_t on top of specified area
  static_assert(std::is_trivially_destructible_v<any_message_t<Protocol>>);
  auto message = reinterpret_cast<const any_message_t<Protocol> *>(first);

  // message type (2 highest bits must be 00, RFC5389, section 6)
  if ((message->type() & 0b1100'0000'0000'0000) != 0)
  {
    error = make_error_code(errc::invalid_message_type);
    return {};
  }

  // message length (must be padded to 4B boundary)
  if (message->length() % 4 != 0)
  {
    error = make_error_code(errc::invalid_message_length);
    return {};
  }
  if (first + Protocol::header_size + message->length() > last)
  {
    error = make_error_code(errc::insufficient_payload_data);
    return {};
  }

  // cookie
  if (message->cookie() != Protocol::cookie)
  {
    error = make_error_code(errc::invalid_message_cookie);
    return {};
  }

  error.clear();
  return message;
}


__turner_end
