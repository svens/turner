#pragma once

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>


__turner_begin


namespace __bits {


const any_attribute_t *find_attribute (
  const uint8_t *first,
  const uint8_t *last,
  uint16_t attribute_type,
  uint16_t integrity_attribute_type,
  size_t padding_size,
  std::error_code &error
) noexcept;


size_t gather_unknown_comprehension_required_or_missing (
  const uint8_t *first,
  const uint8_t *last,
  const uint16_t attributes[],
  size_t attributes_count,
  uint16_t failed[],
  size_t failed_size,
  uint16_t integrity_attribute_type,
  size_t padding_size,
  std::error_code &error
) noexcept;


} // namespace __bits


__turner_end
