#pragma once

/**
 * \file turner/attribute_processor.hpp
 * Attribute reader/writer.
 */


#include <turner/config.hpp>
#include <turner/attribute.hpp>
#include <turner/message.hpp>
#include <sal/byte_order.hpp>
#include <string_view>
#include <utility>


__turner_begin


/**
 * Generic uint32 type attribute reader/writer.
 */
template <typename Protocol>
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
    const any_message_t<Protocol> &,
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
template <typename Protocol>
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
    const any_message_t<Protocol> &,
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
template <typename Protocol>
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
    const any_message_t<Protocol> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    error.clear();
    auto data = reinterpret_cast<const uint8_t *>(attribute.data());
    return {data, data + attribute.length()};
  }
};


__turner_end
