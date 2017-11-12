#pragma once

/**
 * \file turner/attribute_processor.hpp
 * Attribute reader/writer.
 */


#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/__bits/attribute_processor.hpp>


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
   * Attribute value type.
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
