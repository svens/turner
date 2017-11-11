#pragma once

/**
 * \file turner/attribute_processor.hpp
 * Attribute reader/writer.
 */


#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>
#include <sal/net/ip/address.hpp>
#include <string_view>
#include <utility>


__turner_begin


namespace __bits {

uint32_t read_uint32 (
  const any_attribute_t &attribute,
  std::error_code &error
) noexcept;

size_t write_uint32 (
  uint8_t *first,
  uint8_t *last,
  uint32_t value,
  std::error_code &error
) noexcept;

std::string_view read_string (
  const any_attribute_t &attribute,
  std::error_code &error
) noexcept;

size_t write_string (
  uint8_t *first,
  uint8_t *last,
  const std::string_view &value,
  std::error_code &error
) noexcept;

std::pair<const uint8_t *, const uint8_t *> read_array (
  const any_attribute_t &attribute,
  std::error_code &error
) noexcept;

size_t write_array (
  uint8_t *first,
  uint8_t *last,
  const std::pair<const uint8_t *, const uint8_t *> &value,
  std::error_code &error
) noexcept;

error_t read_error (
  const any_attribute_t &attribute,
  std::error_code &error
) noexcept;

size_t write_error (
  uint8_t *first,
  uint8_t *last,
  const error_t &value,
  std::error_code &error
) noexcept;

std::pair<sal::net::ip::address_t, uint16_t> read_address (
  const any_attribute_t &attribute,
  std::error_code &error
) noexcept;

size_t write_address (
  uint8_t *first,
  uint8_t *last,
  const std::pair<sal::net::ip::address_t, uint16_t> &value,
  std::error_code &error
) noexcept;

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
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    return __bits::read_uint32(attribute, error);
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
    return __bits::write_uint32(first, last, value, error);
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
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    return __bits::read_string(attribute, error);
  }


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    return __bits::write_string(first, last, value, error);
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
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    return __bits::read_array(attribute, error);
  }


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    return __bits::write_array(first, last, value, error);
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
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    return __bits::read_error(attribute, error);
  }


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    return __bits::write_error(first, last, value, error);
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
   * \copydoc uint32_attribute_processor_t::read()
   */
  static value_t read (const any_message_t<ProtocolTraits> &,
    const any_attribute_t &attribute,
    std::error_code &error) noexcept
  {
    return __bits::read_address(attribute, error);
  }


  /**
   * \copydoc uint32_attribute_processor_t::write()
   */
  static size_t write (const any_message_t<ProtocolTraits> &,
    uint8_t *first, uint8_t *last,
    const value_t &value,
    std::error_code &error) noexcept
  {
    return __bits::write_address(first, last, value, error);
  }
};


__turner_end
