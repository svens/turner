#pragma once

/**
 * \file turner/msturn/protocol.hpp
 * MS-TURN protocol https://msdn.microsoft.com/en-us/library/cc431507(v=office.12).aspx
 */


#include <turner/config.hpp>
#include <turner/protocol.hpp>
#include <sal/crypto/hash.hpp>
#include <sal/crypto/hmac.hpp>


__turner_begin

namespace msturn {


/**
 * MS-TURN protocol message layout traits.
 *
 * \note For historical reasons (MS-TURN forked from unfinalized TURN draft),
 * Data Indication (0x115) doesn't follow usual message type classification
 * Methods below consider it internally and appear externally as it follows.
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
  static inline constexpr const size_t cookie_offset = 20;


  /**
   * Required cookie content.
   */
  static inline constexpr const std::array<uint8_t, 8> cookie =
  {{
     0x00, 0x0f,                // Type
     0x00, 0x04,                // Length
     0x72, 0xc6, 0x4b, 0xc6     // Cookie
  }};


  /**
   * Offset from beginning of message to transaction ID field.
   */
  static inline constexpr const size_t transaction_id_offset = 4;


  /**
   * Transaction ID size in bytes.
   */
  static inline constexpr const size_t transaction_id_size = 16;


  /**
   * Attributes (TLV) padding boundary.
   */
  static inline constexpr const size_t padding_size = 1;


  /**
   * Message Integrity attribute id.
   */
  static inline constexpr const uint16_t message_integrity = 0x0008;


  /**
   * Padding size for calculating message integrity (0 or 1 for no padding)
   */
  static inline constexpr const size_t message_integrity_padding = 64;


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
    if (type == 0x115)
    {
      return false;
    }
    return (type & 0b0000'0001'0001'0000) == 0b000'0001'0001'0000;
  }


  /**
   * Return true if message \a type is indication.
   */
  static constexpr bool is_indication (uint16_t type) noexcept
  {
    if (type == 0x115)
    {
      return true;
    }
    return (type & 0b0000'0001'0001'0000) == 0b000'0000'0001'0000;
  }
};


/**
 * Structure type describing MSTURN protocol message layout.
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
 * Create HMAC-SHA1 calculator for MS-TURN message where MS-Version is absent
 * or less than 3 (for either side).
 *
 * \see https://msdn.microsoft.com/en-us/library/dd949398(v=office.12).aspx
 */
inline sal::crypto::hmac_t<sal::crypto::sha1> make_integrity_calculator (
  const std::string_view &realm,
  const std::string_view &username,
  const std::string_view &password)
{
  sal::char_array_t<4096> key;
  return sal::crypto::hash_t<sal::crypto::md5>::one_shot(
    key.print(username, ':', realm, ':', password).to_view()
  );
}


/**
 * Create HMAC-SHA256 calculator for MS-TURN message where MS-Version is
 * equal to or greater than 3 (for both sides)
 *
 * \see https://msdn.microsoft.com/en-us/library/dd949398(v=office.12).aspx
 */
inline sal::crypto::hmac_t<sal::crypto::sha256> make_integrity_calculator_v3 (
  const std::string_view &realm,
  const std::string_view &nonce,
  const std::string_view &username,
  const std::string_view &password)
{
  sal::char_array_t<4096> key;
  return sal::crypto::hmac_t<sal::crypto::sha256>::one_shot(
    // K
    sal::crypto::hmac_t<sal::crypto::sha256>::one_shot(nonce, password),
    // Key
    key.print('\1', "TURN", '\0', username, realm, '\0', '\0', '\1', '\0').to_view()
  );
}


} // namespace msturn

__turner_end
