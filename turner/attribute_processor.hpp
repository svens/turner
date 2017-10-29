#pragma once

/**
 * \file turner/attribute_processor.hpp
 * Attribute reader/writer.
 */


#include <turner/config.hpp>
#include <turner/attribute.hpp>
#include <turner/error.hpp>
#include <turner/message.hpp>
#include <sal/byte_order.hpp>
#include <sal/net/ip/address.hpp>
#include <string_view>
#include <utility>


__turner_begin


/**
 * Generic uint32 type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct uint32_attribute_processor_t
{
  /**
   * Attribute value type.
   */
  using value_t = uint32_t;


  /**
   * Read \a attribute value. On failure return default value and set \a error
   * to code describing failure reason.
   */
  static value_t read (
    const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    if (attribute.length() == sizeof(value_t))
    {
      error.clear();
      return sal::network_to_native_byte_order(
        *reinterpret_cast<const uint32_t *>(attribute.data())
      );
    }
    error = make_error_code(errc::unexpected_attribute_length);
    return {};
  }
};


/**
 * Generic string type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct string_attribute_processor_t
{
  /**
   * Attribute value type.
   */
  using value_t = std::string_view;


  /**
   * Read \a attribute value. On failure return default value and set \a error
   * to code describing failure reason.
   */
  static value_t read (
    const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    error.clear();
    return
    {
      reinterpret_cast<const char *>(attribute.data()),
      attribute.length()
    };
  }
};


/**
 * Generic byte array type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct array_attribute_processor_t
{
  /**
   * Attribute value type.
   * - first: beginning of array
   * - second: one byte past end of array
   */
  using value_t = std::pair<const uint8_t *, const uint8_t *>;


  /**
   * Read \a attribute value. On failure return default value and set \a error
   * to code describing failure reason.
   */
  static value_t read (
    const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    error.clear();
    auto data = reinterpret_cast<const uint8_t *>(attribute.data());
    return {data, data + attribute.length()};
  }
};


/**
 * Generic error code type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct error_attribute_processor_t
{
  /**
   * Attribute value type.
   */
  using value_t = error_t;


  /**
   * Read \a attribute value. On failure return default value and set \a error
   * to code describing failure reason.
   */
  static value_t read (
    const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    if (attribute.length() >= 4)
    {
      return error_t(
        (attribute.data()[2] & 0x7) * 100 + attribute.data()[3],
        std::string_view(
          reinterpret_cast<const char *>(attribute.data() + 4),
          attribute.length() - 4
        )
      );
    }
    error = make_error_code(errc::unexpected_attribute_length);
    return {};
  }
};


/**
 * Generic address type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct address_attribute_processor_t
{
  /**
   * Attribute value type.
   * - first: address
   * - second: port in host byte order
   */
  using value_t = std::pair<sal::net::ip::address_t, uint16_t>;


  /**
   * Read \a attribute value. On failure return default value and set \a error
   * to code describing failure reason.
   */
  static value_t read (
    const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error
  ) noexcept;
};


template <typename ProtocolTraits>
typename address_attribute_processor_t<ProtocolTraits>::value_t
  address_attribute_processor_t<ProtocolTraits>::read (
    const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
{
  const auto family = attribute.data()[1];
  if (family == 0x01)
  {
    if (attribute.length() == 8)
    {
      error.clear();
      return
      {
        sal::net::ip::make_address_v4(
          *reinterpret_cast<const sal::net::ip::address_v4_t::bytes_t *>(
            attribute.data() + 4
          )
        ),
        sal::network_to_native_byte_order(
          reinterpret_cast<const uint16_t *>(attribute.data())[1]
        )
      };
    }
    error = make_error_code(errc::unexpected_attribute_length);
  }
  else if (family == 0x02)
  {
    if (attribute.length() == 20)
    {
      error.clear();
      return
      {
        sal::net::ip::make_address_v6(
          *reinterpret_cast<const sal::net::ip::address_v6_t::bytes_t *>(
            attribute.data() + 4
          )
        ),
        sal::network_to_native_byte_order(
          reinterpret_cast<const uint16_t *>(attribute.data())[1]
        )
      };
    }
    error = make_error_code(errc::unexpected_attribute_length);
  }
  else
  {
    error = make_error_code(errc::unexpected_address_family);
  }
  return {};
}


__turner_end
