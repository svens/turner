#pragma once

/**
 * \file turner/stun/protocol.hpp
 * STUN protocol (https://tools.ietf.org/html/rfc5389)
 */


#include <turner/config.hpp>
#include <turner/protocol.hpp>
#include <sal/char_array.hpp>
#include <sal/crypto/hash.hpp>
#include <sal/crypto/hmac.hpp>


__turner_begin

namespace stun {


/**
 * STUN protocol message layout traits.
 */
struct protocol_traits_t
{
  /**
   * Header size in bytes.
   */
  static constexpr size_t header_size = 20;


  /**
   * Offset from beginning of message to cookie field.
   */
  static constexpr size_t cookie_offset = 4;


  /**
   * Required cookie content.
   */
  static constexpr uint32_t cookie = 0x21'12'a4'42;


  /**
   * Offset from beginning of message to transaction ID field.
   */
  static constexpr size_t transaction_id_offset = 8;


  /**
   * Transaction ID size in bytes.
   */
  static constexpr size_t transaction_id_size = 12;


  /**
   * Attributes (TLV) padding boundary.
   */
  static constexpr size_t padding_size = 4;


  /**
   * MESSAGE-INTEGRITY attribute id.
   */
  static constexpr uint16_t message_integrity = 0x0008;


  /**
   * Padding size for calculating message integrity (0 or 1 for no padding)
   */
  static constexpr size_t message_integrity_padding = 1;


  /**
   * Return true if message \a type is request.
   */
  static constexpr bool is_request (uint16_t type) noexcept
  {
    return (type & 0b0000'0001'0001'0000) == 0;
  }


  /**
   * Return true if message \a type is success response.
   */
  static constexpr bool is_success_response (uint16_t type) noexcept
  {
    return (type & 0b0000'0001'0001'0000) == 0b000'0001'0000'0000;
  }


  /**
   * Return true if message \a type is error response.
   */
  static constexpr bool is_error_response (uint16_t type) noexcept
  {
    return (type & 0b0000'0001'0001'0000) == 0b000'0001'0001'0000;
  }


  /**
   * Return true if message \a type is indication.
   */
  static constexpr bool is_indication (uint16_t type) noexcept
  {
    return (type & 0b0000'0001'0001'0000) == 0b000'0000'0001'0000;
  }
};


/**
 * Structure type describing STUN protocol message layout.
 */
using protocol_t = turner::protocol_t<protocol_traits_t>;


/**
 * \copydoc turner::protocol_t::parse(It, It, std::error_code &);
 */
template <typename It>
inline const any_message_t<protocol_traits_t> *parse (It first, It last,
  std::error_code &error) noexcept
{
  return protocol_t::parse(first, last, error);
}


/**
 * \copydoc turner::protocol_t::parse(It, It);
 */
template <typename It>
inline const any_message_t<protocol_traits_t> *parse (It first, It last)
{
  return protocol_t::parse(first, last);
}


/**
 * \copydoc turner::protocol_t::parse(const Data &, std::error_code &);
 */
template <typename Data>
inline const any_message_t<protocol_traits_t> *parse (const Data &data,
  std::error_code &error) noexcept
{
  return protocol_t::parse(data, error);
}


/**
 * \copydoc turner::protocol_t::parse(const Data &);
 */
template <typename Data>
inline const any_message_t<protocol_traits_t> *parse (const Data &data)
{
  return protocol_t::parse(data);
}


/**
 * Message integrity calculator.
 * \see https://tools.ietf.org/html/rfc5389#section-15.4
 */
using integrity_calculator_t = sal::crypto::hmac_t<sal::crypto::sha1>;


/**
 * Create HMAC-SHA1 calculator for short term credentials.
 * \see https://tools.ietf.org/html/rfc5389#section-15.4
 *
 * \note \a password is NOT prepared with SASLprep
 */
inline integrity_calculator_t make_integrity_calculator (
  const std::string_view &password)
{
  return password;
}


/**
 * Create HMAC-SHA1 calculator for long term credentials.
 * \see https://tools.ietf.org/html/rfc5389#section-15.4
 *
 * \note \a password is NOT prepared with SASLprep
 */
inline integrity_calculator_t make_integrity_calculator (
  const std::string_view &realm,
  const std::string_view &username,
  const std::string_view &password)
{
  sal::char_array_t<513 + 1 + 763 + 1 + 1024> input;
  input << username << ':' << realm << ':' << password;
  return sal::crypto::hash_t<sal::crypto::md5>::one_shot(input);
}


} // namespace stun

__turner_end
