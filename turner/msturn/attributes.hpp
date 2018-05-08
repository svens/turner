#pragma once

/**
 * \file turner/msturn/attributes.hpp
 * MS-TURN protocol attributes.
 */


#include <turner/config.hpp>
#include <turner/attribute.hpp>
#include <turner/attribute_processor.hpp>
#include <turner/msturn/protocol.hpp>
#include <turner/stun/attributes.hpp>
#include <turner/turn/attributes.hpp>


__turner_begin

namespace msturn {


/**
 * \defgroup MSTURN_attributes MS-TURN attributes
 * \{
 *
 * \see https://msdn.microsoft.com/en-us/library/dd905469(v=office.12).aspx
 */


// 0x0001 Mapped Address {{{1


/**
 * MS-TURN Mapped Address attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd906841(v=office.12).aspx
 */
using mapped_address_t = protocol_t::reuse_attribute_t<stun::mapped_address_t>;


/**
 * MS-TURN Mapped Address attribute
 * \see https://msdn.microsoft.com/en-us/library/dd906841(v=office.12).aspx
 */
inline constexpr const mapped_address_t mapped_address;


// 0x0006 Username {{{1


/**
 * MS-TURN Username attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd948088(v=office.12).aspx
 */
using username_t = protocol_t::reuse_attribute_t<stun::username_t>;


/**
 * MS-TURN Username attribute
 * \see https://msdn.microsoft.com/en-us/library/dd948088(v=office.12).aspx
 */
inline constexpr const username_t username;


// 0x0009 Error Code {{{1


/**
 * MS-TURN Error Code attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd924136(v=office.12).aspx
 */
using error_code_t = protocol_t::reuse_attribute_t<stun::error_code_t>;


/**
 * MS-TURN Error Code attribute
 * \see https://msdn.microsoft.com/en-us/library/dd924136(v=office.12).aspx
 */
inline constexpr const error_code_t error_code;


// 0x000d Lifetime {{{1


/**
 * MS-TURN Lifetime attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd922471(v=office.12).aspx
 */
using lifetime_t = protocol_t::reuse_attribute_t<turn::lifetime_t>;


/**
 * MS-TURN Lifetime attribute
 * \see https://msdn.microsoft.com/en-us/library/dd922471(v=office.12).aspx
 */
inline constexpr const lifetime_t lifetime;


// 0x000e Alternate Server {{{1


/**
 * MS-TURN Alternate Server attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd925944(v=office.12).aspx
 */
using alternate_server_t = protocol_t::attribute_type_t<0x000e,
  address_attribute_processor_t
>;


/**
 * MS-TURN Alternate Server attribute
 * \see https://msdn.microsoft.com/en-us/library/dd925944(v=office.12).aspx
 */
inline constexpr const alternate_server_t alternate_server;


// 0x0010 Bandwidth {{{1


/**
 * MS-TURN Bandwidth attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd950874(v=office.12).aspx
 */
using bandwidth_t = protocol_t::attribute_type_t<0x0010,
  uint32_attribute_processor_t
>;


/**
 * MS-TURN Bandwidth attribute
 * \see https://msdn.microsoft.com/en-us/library/dd950874(v=office.12).aspx
 */
inline constexpr const bandwidth_t bandwidth;


// 0x0011 Destination Address {{{1


/**
 * MS-TURN Destination Address attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd924543(v=office.12).aspx
 */
using destination_address_t = protocol_t::attribute_type_t<0x0011,
  address_attribute_processor_t
>;


/**
 * MS-TURN Destination Address attribute
 * \see https://msdn.microsoft.com/en-us/library/dd924543(v=office.12).aspx
 */
inline constexpr const destination_address_t destination_address;


// 0x0012 Remote Address {{{1


/**
 * MS-TURN Remote Address attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd948487(v=office.12).aspx
 */
using remote_address_t = protocol_t::attribute_type_t<0x0012,
  address_attribute_processor_t
>;


/**
 * MS-TURN Remote Address attribute
 * \see https://msdn.microsoft.com/en-us/library/dd948487(v=office.12).aspx
 */
inline constexpr const remote_address_t remote_address;


// 0x0013 Data {{{1


/**
 * MS-TURN Data attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd909340(v=office.12).aspx
 */
using data_t = protocol_t::reuse_attribute_t<turn::data_t>;


/**
 * MS-TURN Data attribute
 * \see https://msdn.microsoft.com/en-us/library/dd909340(v=office.12).aspx
 */
inline constexpr const data_t data;


// 0x0014 Nonce {{{1


/**
 * MS-TURN Nonce attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd905560(v=office.12).aspx
 * \note It's value is swapped with STUN REALM
 */
using nonce_t = protocol_t::attribute_type_t<0x0014,
  array_attribute_processor_t
>;


/**
 * MS-TURN Nonce attribute
 * \see https://msdn.microsoft.com/en-us/library/dd905560(v=office.12).aspx
 * \note It's value is swapped with STUN REALM
 */
inline constexpr const nonce_t nonce;


// 0x0015 Realm {{{1


/**
 * MS-TURN Realm attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd905474(v=office.12).aspx
 * \note It's value is swapped with STUN NONCE
 */
using realm_t = protocol_t::attribute_type_t<0x0015,
  string_attribute_processor_t
>;


/**
 * MS-TURN Realm attribute
 * \see https://msdn.microsoft.com/en-us/library/dd905474(v=office.12).aspx
 * \note It's value is swapped with STUN NONCE
 */
inline constexpr const realm_t realm;


// 0x0017 Requested Address Family {{{1


/**
 * MS-TURN Requested Address Family attribute type
 * \see https://msdn.microsoft.com/en-us/library/hh643912(v=office.12).aspx
 */
using requested_address_family_t = protocol_t::reuse_attribute_t<
  turn::requested_address_family_t
>;


/**
 * MS-TURN Requested Address Family attribute
 * \see https://msdn.microsoft.com/en-us/library/hh643912(v=office.12).aspx
 */
inline constexpr const requested_address_family_t requested_address_family;


// 0x8008 MS-Version {{{1


/**
 * MS-TURN MS-Version attribute values
 * \see https://msdn.microsoft.com/en-us/library/dd908334(v=office.12).aspx
 */
enum class protocol_version_t: uint32_t
{
  ice = 1,
  ice2 = 2,
  sha256 = 3,
  ipv6 = 4,
  mturn_udp = 5,
  mturn_tcp = 6,
};


/**
 * MS-TURN attribute MS-Version reader/writer
 */
template <typename ProtocolTraits>
struct protocol_version_attribute_processor_t
{
  /**
   * \copydoc uint32_attribute_processor_t::value_t
   */
  using value_t = protocol_version_t;

  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    return static_cast<value_t>(__bits::read_uint32(attribute, error));
  }

  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    return __bits::write_uint32(first, last,
      static_cast<uint32_t>(value),
      error
    );
  }
};


/**
 * MS-TURN MS-Version attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd908334(v=office.12).aspx
 */
using ms_version_t = protocol_t::attribute_type_t<0x8008,
  protocol_version_attribute_processor_t
>;


/**
 * MS-TURN MS-Version attribute
 * \see https://msdn.microsoft.com/en-us/library/dd908334(v=office.12).aspx
 */
inline constexpr const ms_version_t ms_version;


// 0x8020 XOR Mapped Address {{{1


/**
 * Generic XORed address type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct xor_address_attribute_processor_t
{
  /**
   * Attribute value type.
   * - first: address
   * - second: port in host byte order
   */
  using value_t = std::pair<sal::net::ip::address_t, uint16_t>;


  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &message,
    const any_attribute_t &attribute,
    std::error_code &error
  ) noexcept;


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &message,
    uint8_t *first, uint8_t *last,
    value_t value,
    std::error_code &error
  ) noexcept;
};


/**
 * MS-TURN XOR Mapped Address attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd909268(v=office.12).aspx
 */
using xor_mapped_address_t = protocol_t::attribute_type_t<0x8020,
  xor_address_attribute_processor_t
>;


/**
 * MS-TURN XOR Mapped Address attribute
 * \see https://msdn.microsoft.com/en-us/library/dd909268(v=office.12).aspx
 */
inline constexpr const xor_mapped_address_t xor_mapped_address;


// 0x8022 SOFTWARE {{{1


/**
 * STUN SOFTWARE attribute type
 * (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
using software_t = protocol_t::reuse_attribute_t<stun::software_t>;


/**
 * STUN SOFTWARE attribute
 * (https://tools.ietf.org/html/rfc5389#section-15.10)
 */
inline constexpr const software_t software;


// 0x8032 MS-Alternate Host Name {{{1


/**
 * MS-TURN MS-Alternate Host Name attribute type
 * \see https://msdn.microsoft.com/en-us/library/mt842239(v=office.12).aspx
 */
using ms_alternate_host_name_t = protocol_t::attribute_type_t<0x8032,
  string_attribute_processor_t
>;


/**
 * MS-TURN MS-Alternate Host Name attribute
 * \see https://msdn.microsoft.com/en-us/library/mt842239(v=office.12).aspx
 */
inline constexpr const ms_alternate_host_name_t ms_alternate_host_name;


// 0x8050 MS-Sequence Number {{{1


/**
 * MS-TURN MS-Sequence Number attribute reader/writer
 */
template <typename ProtocolTraits>
struct ms_sequence_number_attribute_processor_t
{
  /**
   * Attribute value type:
   * - first: connection id
   * - second: sequence number
   */
  using value_t = std::pair<std::array<uint8_t, 20>, uint32_t>;

  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    auto [ begin, end ] = __bits::read_array(attribute, error);
    if (!error)
    {
      if (end - begin == sizeof(value_t))
      {
        auto result = *reinterpret_cast<const value_t *>(begin);
        result.second = sal::network_to_native_byte_order(result.second);
        return result;
      }
      error = make_error_code(errc::unexpected_attribute_length);
    }
    return {};
  }

  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    value_t value,
    std::error_code &error) noexcept
  {
    value.second = sal::native_to_network_byte_order(value.second);
    auto payload = reinterpret_cast<const uint8_t *>(&value);
    return __bits::write_array(first, last,
      { payload, payload + sizeof(value) },
      error
    );
  }
};


/**
 * MS-TURN MS-Sequence Number attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd925584(v=office.12).aspx
 */
using ms_sequence_number_t = protocol_t::attribute_type_t<0x8050,
  ms_sequence_number_attribute_processor_t
>;


/**
 * MS-TURN MS-Sequence Number attribute
 * \see https://msdn.microsoft.com/en-us/library/dd925584(v=office.12).aspx
 */
inline constexpr const ms_sequence_number_t ms_sequence_number;


// 0x8055 MS-Service Quality {{{1


/**
 * MS-TURN MS-Service Quality stream type values
 * \see https://msdn.microsoft.com/en-us/library/dd949836(v=office.12).aspx
 */
enum class stream_type_t: uint16_t
{
  audio = 1,
  video = 2,
  supplemental_video = 3,
  data = 4,
};


/**
 * MS-TURN MS-Service Quality type values
 * \see https://msdn.microsoft.com/en-us/library/dd949836(v=office.12).aspx
 */
enum class service_quality_t: uint16_t
{
  best_effort_delivery = 0,
  reliable_delivery = 1,
};


/**
 * MS-TURN MS-Service Quality attribute reader/writer
 * \see https://msdn.microsoft.com/en-us/library/dd949836(v=office.12).aspx
 */
template <typename ProtocolTraits>
struct ms_service_quality_attribute_processor_t
{
  /**
   * \copydoc uint32_attribute_processor_t::value_t
   */
  using value_t = std::pair<stream_type_t, service_quality_t>;

  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    auto [ begin, end ] = __bits::read_array(attribute, error);
    if (!error)
    {
      if (end - begin == sizeof(value_t))
      {
        return type_and_quality(begin, error);
      }
      error = make_error_code(errc::unexpected_attribute_length);
    }
    return {};
  }

  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    uint16_t payload[2] =
    {
      sal::native_to_network_byte_order(static_cast<uint16_t>(value.first)),
      sal::native_to_network_byte_order(static_cast<uint16_t>(value.second)),
    };
    auto p = reinterpret_cast<const uint8_t *>(&payload[0]);
    return __bits::write_array(first, last,
      { p, p + sizeof(payload) },
      error
    );
  }


private:

  static value_t type_and_quality (const uint8_t *payload,
    std::error_code &error) noexcept
  {
    auto p = reinterpret_cast<const uint16_t *>(payload);

    auto stream_type = sal::network_to_native_byte_order(p[0]);
    if (1 > stream_type || stream_type > 4)
    {
      error = make_error_code(errc::unexpected_attribute_value);
      return {};
    }

    auto service_quality = sal::network_to_native_byte_order(p[1]);
    if (service_quality > 1)
    {
      error = make_error_code(errc::unexpected_attribute_value);
      return {};
    }

    return
    {
      static_cast<stream_type_t>(stream_type),
      static_cast<service_quality_t>(service_quality)
    };
  }
};


/**
 * MS-TURN MS-Service Quality attribute type
 * \see https://msdn.microsoft.com/en-us/library/dd949836(v=office.12).aspx
 */
using ms_service_quality_t = protocol_t::attribute_type_t<0x8055,
  ms_service_quality_attribute_processor_t
>;


/**
 * MS-TURN MS-Service Quality attribute
 * \see https://msdn.microsoft.com/en-us/library/dd949836(v=office.12).aspx
 */
inline constexpr const ms_service_quality_t ms_service_quality;


// 0x8090 MS-Alternate Mapped Address {{{1


/**
 * MS-TURN MS-Alternate Mapped Address attribute type
 * \see https://msdn.microsoft.com/en-us/library/hh642281(v=office.12).aspx
 */
using ms_alternate_mapped_address_t = protocol_t::attribute_type_t<0x8090,
  address_attribute_processor_t
>;


/**
 * MS-TURN MS-Alternate Mapped Address attribute
 * \see https://msdn.microsoft.com/en-us/library/hh642281(v=office.12).aspx
 */
inline constexpr const ms_alternate_mapped_address_t ms_alternate_mapped_address;


// 0x8095 Multiplexed TURN Session ID {{{1


/**
 * MS-TURN Multiplexed TURN Session ID attribute reader/writer
 */
template <typename ProtocolTraits>
struct mturn_session_id_attribute_processor_t
{
  /**
   * \copydoc uint32_attribute_processor_t::value_t
   */
  using value_t = uint64_t;

  /**
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    auto [ begin, end ] = __bits::read_array(attribute, error);
    if (!error)
    {
      if (end - begin == sizeof(value_t))
      {
        return *reinterpret_cast<const value_t *>(begin);
      }
      error = make_error_code(errc::unexpected_attribute_length);
    }
    return {};
  }

  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    auto payload = reinterpret_cast<const uint8_t *>(&value);
    return __bits::write_array(first, last,
      { payload, payload + sizeof(value) },
      error
    );
  }
};


/**
 * MS-TURN Multiplexed TURN Session ID attribute type
 * \see https://msdn.microsoft.com/en-us/library/mt757372(v=office.12).aspx
 */
using mturn_session_id_t = protocol_t::attribute_type_t<0x8095,
  mturn_session_id_attribute_processor_t
>;


/**
 * MS-TURN Multiplexed TURN Session ID attribute
 * \see https://msdn.microsoft.com/en-us/library/mt757372(v=office.12).aspx
 */
inline constexpr const mturn_session_id_t mturn_session_id;


// }}}1


/// \}


template <typename ProtocolTraits>
auto xor_address_attribute_processor_t<ProtocolTraits>::read (
  const any_message_t<ProtocolTraits> &message,
  const any_attribute_t &attribute,
  std::error_code &error) noexcept -> value_t
{
  auto [ address, port ] = address_attribute_processor_t<ProtocolTraits>::read(
    message, attribute, error
  );
  if (!error)
  {
    auto &txn_id = message.transaction_id();
    port ^= sal::network_to_native_byte_order(
      *reinterpret_cast<const uint16_t *>(&txn_id[0])
    );

    // IPv4
    if (auto v4 = address.as_v4())
    {
      address = sal::net::ip::make_address_v4(v4->to_uint()
        ^ sal::network_to_native_byte_order(
          *reinterpret_cast<const uint32_t *>(&txn_id[0])
        )
      );
    }

    // IPv6
    else if (auto v6 = address.as_v6())
    {
      auto p = const_cast<uint8_t *>(v6->to_bytes().data());
      for (auto b: txn_id)
      {
        *p++ ^= b;
      }
    }

    return { address, port };
  }
  return {};
}


template <typename ProtocolTraits>
size_t xor_address_attribute_processor_t<ProtocolTraits>::write (
  const any_message_t<ProtocolTraits> &message,
  uint8_t *first, uint8_t *last,
  value_t value,
  std::error_code &error) noexcept
{
  auto &txn_id = message.transaction_id();
  value.second ^= sal::native_to_network_byte_order(
    *reinterpret_cast<const uint16_t *>(&txn_id[0])
  );

  // IPv4
  if (auto v4 = value.first.as_v4())
  {
    value.first = sal::net::ip::make_address_v4(v4->to_uint()
      ^ sal::native_to_network_byte_order(
        *reinterpret_cast<const uint32_t *>(&txn_id[0])
      )
    );
  }

  // IPv6
  else if (auto v6 = value.first.as_v6())
  {
    auto p = const_cast<uint8_t *>(v6->to_bytes().data());
    for (auto b: txn_id)
    {
      *p++ ^= b;
    }
  }

  return turner::__bits::write_address(first, last, value, error);
}


} // namespace msturn

__turner_end
