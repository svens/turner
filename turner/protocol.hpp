#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>
#include <turner/message.hpp>
#include <turner/__bits/helpers.hpp>
#include <sal/byte_order.hpp>
#include <sal/crypto/random.hpp>
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
   * Protocol name is defined, if \c "void operator>> (ProtocolTraits, const
   * char *&name)" is provided. On operator invocation, name should be
   * assigned to protocol name.
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
    auto begin = __bits::to_cptr(first);
    auto end = begin + (last - first) * sizeof(*first);
    return parse(begin, end, error);
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
   * Return message writer object for \a MessageType. Returned object is
   * allowed to write attributes into memory area [\a first, \a last).
   *
   * This method builds immediately message header but it does not add any
   * attributes nor set valid message length. Use returned object to add
   * attributes and finalize message.
   *
   * On error, set \a error and return undefined message_writer_t object.
   */
  template <uint16_t MessageType, typename It>
  static message_writer_t<ProtocolTraits, MessageType> build (
    message_type_t<MessageType> message_type,
    It first,
    It last,
    std::error_code &error) noexcept
  {
    static_assert(message_type.is_request() || message_type.is_indication(),
      "expected request or indication message type"
    );
    auto begin = __bits::to_ptr(first);
    auto end = begin + (last - first) * sizeof(*first);
    if (build(MessageType, begin, end, error))
    {
      return {begin, end};
    }
    return {nullptr, nullptr};
  }


  /**
   * Return message writer object for \a MessageType. Returned object is
   * allowed to write attributes into memory area [\a first, \a last).
   *
   * This method builds immediately message header but it does not add any
   * attributes nor set valid message length. Use returned object to add
   * attributes and finalize message.
   *
   * \throws std::system_error on message header building failure.
   */
  template <uint16_t MessageType, typename It>
  static message_writer_t<ProtocolTraits, MessageType> build (
    message_type_t<MessageType> message_type,
    It first,
    It last)
  {
    return build(message_type, first, last,
      sal::throw_on_error("protocol::build")
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

  static_assert(std::is_trivially_destructible_v<message_t>);

  static_assert(
    (traits_t::padding_size & (traits_t::padding_size - 1)) == 0,
    "expected value of traits_t::padding_size to be power of 2"
  );

  static const message_t *parse (
    const uint8_t *first,
    const uint8_t *last,
    std::error_code &error
  ) noexcept;

  static bool build (
    uint16_t message_type,
    uint8_t *first,
    uint8_t *last,
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
  if (first + traits_t::header_size > last)
  {
    error = make_error_code(errc::insufficient_header_data);
    return {};
  }

  // passed initial checks, overlay any_message_t on top of specified area
  auto message = reinterpret_cast<const message_t *>(first);

  // message type
  if ((message->type() & __bits::method_mask) != 0)
  {
    error = make_error_code(errc::invalid_message_type);
    return {};
  }

  // message length
  if constexpr (ProtocolTraits::padding_size > 1)
  {
    if (message->length() % ProtocolTraits::padding_size != 0)
    {
      error = make_error_code(errc::invalid_message_length);
      return {};
    }
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


template <typename ProtocolTraits>
bool protocol_t<ProtocolTraits>::build (uint16_t message_type,
  uint8_t *first,
  uint8_t *last,
  std::error_code &error) noexcept
{
  if (first + traits_t::header_size <= last)
  {
    // message type
    reinterpret_cast<uint16_t *>(first)[0] =
      sal::native_to_network_byte_order(message_type);

    // message length
    reinterpret_cast<uint16_t *>(first)[1] = 0;

    // cookie
    *reinterpret_cast<cookie_t *>(first + ProtocolTraits::cookie_offset) =
      ProtocolTraits::cookie;

    // transaction id
    auto p = first + ProtocolTraits::transaction_id_offset;
    sal::crypto::random(p, p + ProtocolTraits::transaction_id_size);

    error.clear();
    return true;
  }

  error = make_error_code(errc::not_enough_room);
  return false;
}


__turner_end
