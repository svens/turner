#include <turner/stun/attributes.hpp>
#include <turner/__bits/attribute_processor.hpp>

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

    // IPv4
    if (auto v4 = address.as_v4())
    {
      address = sal::net::ip::make_address_v4(v4->to_uint() ^ xor_cookie_32);
    }

    // IPv6
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


size_t xor_address_attribute_processor_t::write (
  const any_message_t<protocol_traits_t> &message,
  uint8_t *first, uint8_t *last,
  const value_t &value,
  std::error_code &error) noexcept
{
  value_t xor_value = value;
  xor_value.second ^= xor_cookie_16;

  // IPv4
  if (auto v4 = xor_value.first.as_v4())
  {
    xor_value.first = sal::net::ip::make_address_v4(
      v4->to_uint() ^ xor_cookie_32
    );
  }

  // IPv6
  else if (auto v6 = xor_value.first.as_v6())
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

  return turner::__bits::write_address(first, last, xor_value, error);
}


} // namespace stun


__turner_end
