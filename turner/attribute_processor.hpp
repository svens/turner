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


namespace __bits {

inline bool has_enough_room (const uint8_t *first, const uint8_t *last,
  size_t required_size,
  std::error_code &error) noexcept
{
  if (first + required_size <= last)
  {
    error.clear();
    return true;
  }
  error = make_error_code(errc::not_enough_room);
  return false;
}


template <typename It>
inline auto make_output_iterator (It first, It) noexcept
{
#if defined(_MSC_VER)
  return stdext::make_unchecked_array_iterator(first);
#else
  return first;
#endif
}


} // namespace __bits


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


  /**
   * Write \a value into [\a first, \a last). If \a value does not fit into
   * specified region, \a error is set to errc::not_enough_room and nothing
   * is writtern. Returns \a value size (without possible padding roundup).
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    if (__bits::has_enough_room(first, last, sizeof(value_t), error))
    {
      *reinterpret_cast<uint32_t *>(first) =
        sal::native_to_network_byte_order(value);
    }
    return sizeof(value_t);
  }
};


/**
 * Generic string type attribute reader/writer.
 */
template <typename ProtocolTraits>
struct string_attribute_processor_t
{
  /**
   * \copydoc uint32_attribute_processor_t::value_t
   */
  using value_t = std::string_view;


  /**
   * \copydoc uint32_attribute_processor_t::read()
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


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    auto required_size = (value.size() + 3) & ~3;
    if (__bits::has_enough_room(first, last, required_size, error))
    {
      std::uninitialized_copy(value.begin(), value.end(),
        __bits::make_output_iterator(first, last)
      );
    }
    return value.size();
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
   * \copydoc uint32_attribute_processor_t::read()
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


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    auto required_size = (value.second - value.first + 3) & ~3;
    if (__bits::has_enough_room(first, last, required_size, error))
    {
      std::uninitialized_copy(value.first, value.second,
        __bits::make_output_iterator(first, last)
      );
    }
    return value.second - value.first;
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
   * \copydoc uint32_attribute_processor_t::read()
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


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    auto required_size = sizeof(uint32_t) + ((value.message.size() + 3) & ~3);
    if (__bits::has_enough_room(first, last, required_size, error))
    {
      *first++ = 0;
      *first++ = 0;
      *first++ = static_cast<uint8_t>(value.code/100);
      *first++ = static_cast<uint8_t>(value.code%100);
      std::uninitialized_copy(value.message.begin(), value.message.end(),
        __bits::make_output_iterator(first, last)
      );
    }
    return sizeof(uint32_t) + value.message.size();
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
