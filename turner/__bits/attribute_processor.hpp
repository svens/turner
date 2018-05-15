#pragma once

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

void read_address (
  const any_attribute_t &attribute,
  sal::net::ip::address_t *address,
  uint16_t *port,
  std::error_code &error
) noexcept;

size_t write_address (
  uint8_t *first,
  uint8_t *last,
  const sal::net::ip::address_t &address,
  uint16_t port,
  std::error_code &error
) noexcept;


} // namespace __bits


__turner_end
