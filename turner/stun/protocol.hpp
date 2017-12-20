#pragma once

/**
 * \file turner/stun/protocol.hpp
 * STUN protocol (https://tools.ietf.org/html/rfc5389)
 */


#include <turner/config.hpp>
#include <turner/protocol.hpp>
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
  static inline constexpr const size_t header_size = 20;


  /**
   * Offset from beginning of message to cookie field.
   */
  static inline constexpr const size_t cookie_offset = 4;


  /**
   * Required cookie content.
   */
  static inline constexpr const std::array<uint8_t, 4> cookie =
  {
    { 0x21, 0x12, 0xa4, 0x42 }
  };


  /**
   * Offset from beginning of message to transaction ID field.
   */
  static inline constexpr const size_t transaction_id_offset = 8;


  /**
   * Transaction ID size in bytes.
   */
  static inline constexpr const size_t transaction_id_size = 12;


  /**
   * Attributes (TLV) padding boundary.
   */
  static inline constexpr const size_t padding_size = 4;


  /**
   * MESSAGE-INTEGRITY attribute id.
   */
  static inline constexpr const uint16_t message_integrity = 0x0008;


  /**
   * Padding size for calculating message integrity (0 or 1 for no padding)
   */
  static inline constexpr const size_t message_integrity_padding = 1;


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
 * \copydoc turner::protocol_t::parse(It,It,std::error_code&);
 */
template <typename It>
inline const any_message_t<protocol_traits_t> *parse (It first, It last,
  std::error_code &error) noexcept
{
  return protocol_t::parse(first, last, error);
}


/**
 * \copydoc turner::protocol_t::parse(It,It);
 */
template <typename It>
inline const any_message_t<protocol_traits_t> *parse (It first, It last)
{
  return protocol_t::parse(first, last);
}


/**
 * Create HMAC-SHA1 calculator for short term credentials.
 * \see https://tools.ietf.org/html/rfc5389#section-15.4
 *
 * \note \a password is NOT prepared with SASLprep
 */
inline sal::crypto::hmac_t<sal::crypto::sha1> make_integrity_calculator (
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
inline sal::crypto::hmac_t<sal::crypto::sha1> make_integrity_calculator (
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
