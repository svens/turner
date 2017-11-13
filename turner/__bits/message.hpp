#pragma once

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>


__turner_begin


namespace __bits {


const any_attribute_t *find_attribute (
  const uint8_t *first, const uint8_t *last,
  uint16_t attribute_type,
  std::error_code &error
) noexcept;


} // namespace __bits


__turner_end
