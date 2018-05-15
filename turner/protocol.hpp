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


namespace __bits {

template <typename T, typename = std::void_t<>>
constexpr bool has_stream_framing_header = false;

template <typename T>
constexpr bool has_stream_framing_header<T, std::void_t<typename T::stream_framing_header_t>> = true;

} // namespace __bits


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
  using cookie_t = std::remove_const_t<decltype(traits_t::cookie)>;

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
   * True if ProtocolTraits defines type \c framing_header_t which deals with
   * stream-based transport framing header. If such type is not defined,
   * protocol does not use framing header.
   */
  static constexpr bool has_stream_framing_header =
    __bits::has_stream_framing_header<traits_t>;


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
      return static_cast<uint16_t>(sizeof(cookie_t));
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
    if constexpr (is_msvc_compiler && is_debug_build)
    {
      if (first == last)
      {
        error = make_error_code(errc::insufficient_header_data);
        return {};
      }
    }

    // region length
    auto begin = sal::to_ptr(first);
    auto end = sal::to_end_ptr(first, last);
    if (begin + header_and_cookie_size() > end)
    {
      error = make_error_code(errc::insufficient_header_data);
      return {};
    }

    // message type
    auto message = reinterpret_cast<const any_message_t *>(begin);
    if ((message->type() & 0b1100'0000'0000'0000) != 0)
    {
      error = make_error_code(errc::invalid_message_type);
      return {};
    }

    // cookie
    auto cookie_p = reinterpret_cast<const cookie_t *>(
      begin + traits_t::cookie_offset
    );
    if (*cookie_p != sal::native_to_network_byte_order(traits_t::cookie))
    {
      error = make_error_code(errc::invalid_message_cookie);
      return {};
    }

    // payload length
    auto payload_length = message->length();
    if constexpr (traits_t::padding_size > 1)
    {
      if (payload_length % traits_t::padding_size != 0)
      {
        error = make_error_code(errc::invalid_message_length);
        return {};
      }
    }
    if (begin + traits_t::header_size + payload_length > end)
    {
      error = make_error_code(errc::insufficient_payload_data);
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
