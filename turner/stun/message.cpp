#include <turner/stun/message.hpp>


__turner_begin


// explicitly instantiate STUN message parser
template
basic_message_t<stun::protocol_t> basic_message_t<stun::protocol_t>::make (
  const uint8_t *first,
  const uint8_t *last,
  std::error_code &error
) noexcept;


__turner_end
