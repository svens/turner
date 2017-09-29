#include <turner/stun/message.hpp>


__turner_begin


namespace stun {


basic_message_t<protocol_t> protocol_t::message (const uint8_t *first,
  const uint8_t *last,
  std::error_code &error) noexcept
{
  error.clear();
  return basic_message_t<protocol_t>(first, last);
}


} // namespace stun


__turner_end
