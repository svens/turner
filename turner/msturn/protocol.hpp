#pragma once

/**
 * \file turner/msturn/protocol.hpp
 * MS-TURN protocol https://msdn.microsoft.com/en-us/library/cc431507(v=office.12).aspx
 */


#include <turner/config.hpp>
#include <turner/protocol.hpp>
#include <sal/byte_order.hpp>
#include <sal/char_array.hpp>
#include <sal/crypto/hash.hpp>
#include <sal/crypto/hmac.hpp>


__turner_begin

namespace msturn {


/**
 * MS-TURN protocol message layout traits.
 *
 * \note For historical reasons (MS-TURN forked from unfinalized TURN draft),
 * Data Indication (0x115) doesn't follow usual message type classification.
 * This class' methods consider it internally and appear externally as it
 * follows.
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
  static constexpr size_t cookie_offset = 20;


  /**
   * Required cookie content.
   *
   * \note MSTURN documentation requires Magic Cookie to be first attribute in
   * payload. For uniform handling like STUN/TURN, library treats it as 8B
   * value at first attribute offset.
   */
  static constexpr uint64_t cookie = 0x00'0f'00'04'72'c6'4b'c6;


  /**
   * Offset from beginning of message to transaction ID field.
   */
  static constexpr size_t transaction_id_offset = 4;


  /**
   * Transaction ID size in bytes.
   */
  static constexpr size_t transaction_id_size = 16;


  /**
   * Attributes (TLV) padding boundary.
   */
  static constexpr size_t padding_size = 1;


  /**
   * Message Integrity attribute id.
   */
  static constexpr uint16_t message_integrity = 0x0008;


  /**
   * Padding size for calculating message integrity (0 or 1 for no padding)
   */
  static constexpr size_t message_integrity_padding = 64;


  /**
   * Stream-based transport framing header data structure.
   * \note Instances of this class is meant to be overlayed directly onto raw
   * data and methods work relative to \c this.
   */
  struct stream_framing_header_t
  {
    /**
     * Framing header size
     */
    static constexpr size_t size = 4;


    /**
     * Type of data following framing header.
     */
    enum class type_t: uint8_t
    {
      control_message = 0x02,   ///< MS-TURN control message
      end_to_end_data = 0x03,   ///< End-to-end data
    };


    stream_framing_header_t () noexcept = default;


    /**
     * Construct new framing header at \a this using \a type and \a length for
     * following data.
     */
    stream_framing_header_t (type_t type, size_t length) noexcept
    {
      reinterpret_cast<type_t *>(this)[0] = type;
      reinterpret_cast<uint8_t *>(this)[1] = 0;
      reinterpret_cast<uint16_t *>(this)[1] =
        sal::native_to_network_byte_order(static_cast<uint16_t>(length));
    }


    /**
     * Return type of data following after framing header.
     */
    type_t type () const noexcept
    {
      return reinterpret_cast<const type_t *>(this)[0];
    }


    /**
     * Return length of data following framing header.
     */
    size_t length () const noexcept
    {
      return sal::network_to_native_byte_order(
        reinterpret_cast<const uint16_t *>(this)[1]
      );
    }
  };


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
 * \copydoc turner::protocol_t::parse(It, It, std::error_code&);
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
