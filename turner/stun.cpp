#include <turner/stun.hpp>


__turner_begin

namespace stun {


namespace {

// see protocol_traits_t::cookie
constexpr const uint16_t xor_cookie_16 = 0x2112;
constexpr const uint32_t xor_cookie_32 = 0x2112a442;

} // namespace


xor_address_attribute_processor_t::value_t xor_address_attribute_processor_t::read (
  const any_message_t<protocol_traits_t> &message,
  const any_attribute_t &attribute,
  std::error_code &error) noexcept
{
  auto [ address, port ] = address_attribute_processor_t<protocol_traits_t>::read(
    message, attribute, error
  );
  if (!error)
  {
    port ^= xor_cookie_16;
    if (auto v4 = address.as_v4())
    {
      address = sal::net::ip::make_address_v4(v4->to_uint() ^ xor_cookie_32);
    }
    else if (auto v6 = address.as_v6())
    {
      auto p = const_cast<uint8_t *>(v6->to_bytes().data());
      for (auto b: protocol_traits_t::cookie)
      {
        *p++ ^= b;
      }
      for (auto b: message.transaction_id())
      {
        *p++ ^= b;
      }
    }
    return { address, port };
  }
  return {};
}


} // namespace stun

__turner_end
