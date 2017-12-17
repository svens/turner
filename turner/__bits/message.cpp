#include <turner/__bits/message.hpp>
#include <turner/attribute.hpp>


__turner_begin


namespace __bits {


namespace {

constexpr const size_t type_and_length = 2 * sizeof(uint16_t);

inline constexpr size_t round_up (size_t size, size_t padding_size)
  noexcept
{
  return type_and_length + ((size + padding_size - 1) & ~(padding_size - 1));
}

} // namespae


const any_attribute_t *find_attribute (
  const uint8_t *first, const uint8_t *last,
  uint16_t attribute_type,
  size_t padding_size,
  std::error_code &error) noexcept
{
  while (first + type_and_length <= last)
  {
    auto attribute = reinterpret_cast<const any_attribute_t *>(first);
    if (attribute->type() == attribute_type)
    {
      if (first + round_up(attribute->length(), padding_size) <= last)
      {
        return attribute;
      }
      else
      {
        error = make_error_code(errc::insufficient_payload_data);
        return {};
      }
    }
    first += round_up(attribute->length(), padding_size);
  }

  error = make_error_code(errc::attribute_not_found);
  return {};
}


} // namespace __bits


__turner_end
