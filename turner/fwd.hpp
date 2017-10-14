#pragma once

/**
 * \file turner/fwd.hpp
 * Forward declarations
 */

#include <turner/config.hpp>


__turner_begin


namespace __bits {

// RFC5398, 6
__turner_inline_var constexpr uint16_t
  class_mask =             0b000'0001'0001'0000,
  indication_class =       0b000'0000'0001'0000,
  success_response_class = 0b000'0001'0000'0000,
  error_response_class =   0b000'0001'0001'0000;

} // namespace __bits


// turner/protocol.hpp
template <typename Protocol, const char *Name = nullptr>
class protocol_t;

// turner/message_type.hpp
template <typename Protocol, uint16_t Type, const char *Name = nullptr>
class message_type_t;


__turner_end
