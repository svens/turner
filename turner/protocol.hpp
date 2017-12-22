#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/error.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <array>
#include <iterator>
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
  using message_type_t = turner::message_type_t<traits_t, MessageType>;

  /**
   * Untyped message as defined by protocol.
   */
  using any_message_t = turner::any_message_t<traits_t>;

  /**
   * Attribute type as defined by protocol.
   */
  template <
    uint16_t AttributeType,
    template <typename> typename AttributeProcessor
  >
  using attribute_type_t = turner::attribute_type_t<
    traits_t,
    AttributeType,
    AttributeProcessor
  >;

  /**
   * Pull attribute from different protocol into this protocol i.e. reuse
   * attribute type value and processor but this protocol's traits.
   */
  template <typename Attribute>
  using reuse_attribute_t = attribute_type_t<
    Attribute::type,
    Attribute::template rebind_processor_t
  >;


  /**
   * Returns minimum message length (in bytes) for payload. Returned value is
   * size of cookie if it is part of payload. Otherwise, if cookie is part of
   * header, returns zero.
   */
  static constexpr uint16_t min_payload_length () noexcept
  {
    if constexpr (traits_t::header_size > traits_t::cookie_offset)
    {
      return 0;
    }
    else
    {
      return static_cast<uint16_t>(traits_t::cookie.size());
    }
  }


  /**
   * Return minimum required size (in bytes) for message that includes header
   * and cookie (either part of header or payload).
   */
  static constexpr size_t header_and_cookie_size () noexcept
  {
    return traits_t::header_size + min_payload_length();
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
  static const any_message_t *parse (It first, It last,
    std::error_code &error) noexcept
  {
    if (first == last)
    {
      error = make_error_code(errc::insufficient_header_data);
      return {};
    }

    auto begin = sal::to_ptr(first);
    auto end = sal::to_end_ptr(first, last);
    if (begin + header_and_cookie_size() > end)
    {
      error = make_error_code(errc::insufficient_header_data);
      return {};
    }

    auto message = reinterpret_cast<const any_message_t *>(begin);
    if (!is_valid_message_type(message->type()))
    {
      error = make_error_code(errc::invalid_message_type);
      return {};
    }

    if constexpr (traits_t::padding_size > 1)
    {
      if (message->length() % traits_t::padding_size != 0)
      {
        error = make_error_code(errc::invalid_message_length);
        return {};
      }
    }
    if (begin + traits_t::header_size + message->length() > end)
    {
      error = make_error_code(errc::insufficient_payload_data);
      return {};
    }

    if (message->cookie() != traits_t::cookie)
    {
      error = make_error_code(errc::invalid_message_cookie);
      return {};
    }

    error.clear();
    return message;
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
  static const any_message_t *parse (It first, It last)
  {
    return parse(first, last, sal::throw_on_error("protocol::parse"));
  }


  /**
   * Return pointer to message instance wrapping raw network format in
   * \a data.
   *
   * Returned pointed object is not actually instantiated but overlayed onto
   * specified memory occupied by \a data. Each time any of any_message_t
   * method is invocated, it parses fields relative to \a this.
   *
   * If during call to parse() message validation fails, nullptr is
   * returned and error is set to code describing failure.
   */
  template <typename Data>
  static const any_message_t *parse (const Data &data, std::error_code &error)
    noexcept
  {
    using std::cbegin;
    using std::cend;
    return parse(cbegin(data), cend(data), error);
  }


  /**
   * Return pointer to message instance wrapping raw network format in
   * \a data.
   *
   * Returned pointed object is not actually instantiated but overlayed onto
   * specified memory occupied by \a data. Each time any of any_message_t
   * method is invocated, it parses fields relative to \a this.
   *
   * \throws std::system_error if during call to parse() message
   * validation fails.
   */
  template <typename Data>
  static const any_message_t *parse (const Data &data)
  {
    return parse(data, sal::throw_on_error("protocol::parse"));
  }


private:

  static_assert(std::is_trivially_destructible_v<any_message_t>);

  static_assert(
    (traits_t::padding_size & (traits_t::padding_size - 1)) == 0,
    "expected value of traits_t::padding_size to be power of 2"
  );
};


__turner_end
