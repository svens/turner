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
 * Concrete protocols define own ProtocolTraits and wrap it into
 * protocol_t which provides common API for message parsing.
 */
template <typename ProtocolTraits>
class protocol_t
{
public:

  /**
   * Protocol traits.
   */
  using traits_t = ProtocolTraits;

  /**
   * Cookie type for protocol message.
   */
  using cookie_t = std::array<uint8_t, traits_t::cookie.max_size()>;

  /**
   * Transaction ID type for protocol message.
   */
  using transaction_id_t = std::array<uint8_t, traits_t::transaction_id_size>;

  /**
   * Message type as defined by protocol
   */
  template <uint16_t MessageType>
  using message_type_t = turner::message_type_t<ProtocolTraits, MessageType>;

  /**
   * Untyped message as defined by protocol.
   */
  using message_t = any_message_t<traits_t>;


  /**
   * Return protocol name (if defined).
   *
   * Protocol name is defined, if \c "void operator>> (ProtocolTraits, const char *&name)"
   * is provided. On operator invocation, name should be assigned to protocol
   * name.
   *
   * If this operator is not provided, nullptr is returned.
   */
  static constexpr const char *name () noexcept
  {
    if constexpr (__bits::has_name_getter_v<protocol_t>)
    {
      const char *result{};
      protocol_t{} >> result;
      return result;
    }
    else
    {
      return nullptr;
    }
  }


  /**
   * Return pointer to message instance wrapping raw network format in range
   * [\a first, \a last).
   *
   * Returned pointed object is not actually instantiated but overlayed onto
   * specified memory range [\a first, \a last). Each time any of
   * any_message_t method is invocated, it parses fields relative to \a this.
   *
   * If during call to parse() message validation fails, nullptr is
   * returned and error is set to code describing failure.
   */
  template <typename It>
  static const message_t *parse (It first, It last,
    std::error_code &error) noexcept
  {
    auto begin = __bits::to_ptr(first);
    return parse(begin, begin + (last - first), error);
  }


  /**
   * Return pointer to message instance wrapping raw network format in range
   * [\a first, \a last).
   *
   * Returned pointed object is not actually instantiated but overlayed onto
   * specified memory range [\a first, \a last). Each time any of
   * any_message_t method is invocated, it parses fields relative to \a this.
   *
   * \throws std::system_error if during call to parse() message
   * validation fails.
   */
  template <typename It>
  static const message_t *parse (It first, It last)
  {
    return parse(first, last,
      sal::throw_on_error("protocol::parse")
    );
  }


  /**
   * Write to \a stream protocol name. If name is not defined, nothing is
   * written.
   */
  friend std::ostream &operator<< (std::ostream &stream, protocol_t)
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

  static const message_t *parse (
    const uint8_t *first,
    const uint8_t *last,
    std::error_code &error
  ) noexcept;
};


template <typename ProtocolTraits>
const typename protocol_t<ProtocolTraits>::message_t *
  protocol_t<ProtocolTraits>::parse (
    const uint8_t *first,
    const uint8_t *last,
    std::error_code &error) noexcept
{
  // validate arguments
  if (size_t(last - first) < traits_t::header_size)
  {
    error = make_error_code(errc::insufficient_header_data);
    return {};
  }

  // passed initial checks, overlay any_message_t on top of specified area
  static_assert(std::is_trivially_destructible_v<message_t>);
  auto message = reinterpret_cast<const message_t *>(first);

  // message type
  if ((message->type() & __bits::method_mask) != 0)
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
  if (first + traits_t::header_size + message->length() > last)
  {
    error = make_error_code(errc::insufficient_payload_data);
    return {};
  }

  // cookie
  if (message->cookie() != traits_t::cookie)
  {
    error = make_error_code(errc::invalid_message_cookie);
    return {};
  }

  error.clear();
  return message;
}


__turner_end
