#include <turner/attribute_processor.hpp>
#include <turner/attribute.hpp>
#include <turner/message.hpp>
#include <sal/byte_order.hpp>


__turner_begin


namespace __bits {


namespace {

inline bool has_enough_room (const uint8_t *first, const uint8_t *last,
  size_t required_size, std::error_code &error) noexcept
{
  if (first + required_size <= last)
  {
    error.clear();
    return true;
  }
  error = make_error_code(errc::not_enough_room);
  return false;
}

template <typename It>
inline auto make_output_iterator (It first, It) noexcept
{
#if defined(_MSC_VER)
  return stdext::make_unchecked_array_iterator(first);
#else
  return first;
#endif
}

} // namespace


uint32_t read_uint32 (const any_attribute_t &attribute,
  std::error_code &error) noexcept
{
  if (attribute.length() == sizeof(uint32_t))
  {
    error.clear();
    return sal::network_to_native_byte_order(
      *reinterpret_cast<const uint32_t *>(attribute.data())
    );
  }
  error = make_error_code(errc::unexpected_attribute_length);
  return {};
}


size_t write_uint32 (uint8_t *first, uint8_t *last, uint32_t value,
  std::error_code &error) noexcept
{
  if (has_enough_room(first, last, sizeof(value), error))
  {
    *reinterpret_cast<uint32_t *>(first) =
      sal::native_to_network_byte_order(value);
  }
  return sizeof(value);
}


std::string_view read_string (const any_attribute_t &attribute,
  std::error_code &error) noexcept
{
  error.clear();
  return
  {
    reinterpret_cast<const char *>(attribute.data()),
    attribute.length()
  };
}


size_t write_string (uint8_t *first, uint8_t *last,
  const std::string_view &value,
  std::error_code &error) noexcept
{
  auto required_size = (value.size() + 3) & ~3;
  if (has_enough_room(first, last, required_size, error))
  {
    std::uninitialized_copy(value.begin(), value.end(),
      __bits::make_output_iterator(first, last)
    );
  }
  return value.size();
}


std::pair<const uint8_t *, const uint8_t *> read_array (
  const any_attribute_t &attribute,
  std::error_code &error) noexcept
{
  error.clear();
  auto data = reinterpret_cast<const uint8_t *>(attribute.data());
  return {data, data + attribute.length()};
}


size_t write_array (uint8_t *first, uint8_t *last,
  const std::pair<const uint8_t *, const uint8_t *> &value,
  std::error_code &error) noexcept
{
  auto required_size = (value.second - value.first + 3) & ~3;
  if (has_enough_room(first, last, required_size, error))
  {
    std::uninitialized_copy(value.first, value.second,
      __bits::make_output_iterator(first, last)
    );
  }
  return value.second - value.first;
}


error_t read_error (const any_attribute_t &attribute,
  std::error_code &error) noexcept
{
  if (attribute.length() >= 4)
  {
    return error_t(
      (attribute.data()[2] & 0x7) * 100 + attribute.data()[3],
      std::string_view(
        reinterpret_cast<const char *>(attribute.data() + 4),
        attribute.length() - 4
      )
    );
  }
  error = make_error_code(errc::unexpected_attribute_length);
  return {};
}


size_t write_error (uint8_t *first, uint8_t *last, const error_t &value,
  std::error_code &error) noexcept
{
  auto required_size = sizeof(uint32_t) + ((value.message.size() + 3) & ~3);
  if (has_enough_room(first, last, required_size, error))
  {
    *first++ = 0;
    *first++ = 0;
    *first++ = static_cast<uint8_t>(value.code/100);
    *first++ = static_cast<uint8_t>(value.code%100);
    std::uninitialized_copy(value.message.begin(), value.message.end(),
      __bits::make_output_iterator(first, last)
    );
  }
  return sizeof(uint32_t) + value.message.size();
}


std::pair<sal::net::ip::address_t, uint16_t> read_address (
  const any_attribute_t &attribute,
  std::error_code &error) noexcept
{
  if (attribute.length() == 0)
  {
    error = make_error_code(errc::unexpected_attribute_length);
    return {};
  }

  // IPv4
  const auto family = attribute.data()[1];
  if (family == 0x01)
  {
    if (attribute.length() == 8)
    {
      error.clear();
      return
      {
        sal::net::ip::make_address_v4(
          *reinterpret_cast<const sal::net::ip::address_v4_t::bytes_t *>(
            attribute.data() + 4
          )
        ),
        sal::network_to_native_byte_order(
          reinterpret_cast<const uint16_t *>(attribute.data())[1]
        )
      };
    }
    error = make_error_code(errc::unexpected_attribute_length);
  }

  // IPv6
  else if (family == 0x02)
  {
    if (attribute.length() == 20)
    {
      error.clear();
      return
      {
        sal::net::ip::make_address_v6(
          *reinterpret_cast<const sal::net::ip::address_v6_t::bytes_t *>(
            attribute.data() + 4
          )
        ),
        sal::network_to_native_byte_order(
          reinterpret_cast<const uint16_t *>(attribute.data())[1]
        )
      };
    }
    error = make_error_code(errc::unexpected_attribute_length);
  }

  // unknown
  else
  {
    error = make_error_code(errc::unexpected_address_family);
  }

  return {};
}


size_t write_address (uint8_t *first, uint8_t *last,
  const std::pair<sal::net::ip::address_t, uint16_t> &value,
  std::error_code &error) noexcept
{
  // IPv4
  if (auto v4 = value.first.as_v4())
  {
    constexpr const size_t required_size = 8;
    if (has_enough_room(first, last, required_size, error))
    {
      *first++ = 0x00;
      *first++ = 0x01;

      *reinterpret_cast<uint16_t *>(first) =
        sal::native_to_network_byte_order(value.second);
      first += 2;

      *reinterpret_cast<uint32_t *>(first) =
        sal::native_to_network_byte_order(v4->to_uint());
    }
    return required_size;
  }

  // IPv6
  else if (auto v6 = value.first.as_v6())
  {
    constexpr const size_t required_size = 20;
    if (has_enough_room(first, last, required_size, error))
    {
      *first++ = 0x00;
      *first++ = 0x02;

      *reinterpret_cast<uint16_t *>(first) =
        sal::native_to_network_byte_order(value.second);
      first += 2;

      *reinterpret_cast<sal::net::ip::address_v6_t::bytes_t *>(first) =
        v6->to_bytes();
    }
    return required_size;
  }

  // unknown
  error = make_error_code(errc::unexpected_address_family);
  return 0;
}


} // namespace __bits


__turner_end
