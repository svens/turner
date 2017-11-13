#include <turner/__bits/message.hpp>
#include <turner/attribute.hpp>


__turner_begin


namespace __bits {


namespace {

constexpr const size_t type_and_length = 2 * sizeof(uint16_t);

} // namespae


const any_attribute_t *find_attribute (
  const uint8_t *first, const uint8_t *last,
  uint16_t attribute_type,
  std::error_code &error) noexcept
{
  while (first + type_and_length <= last)
  {
    auto attribute = reinterpret_cast<const any_attribute_t *>(first);
    if (attribute->type() == attribute_type)
    {
      if (first + attribute->size() <= last)
      {
        return attribute;
      }
      else
      {
        error = make_error_code(errc::unexpected_attribute_length);
        return {};
      }
    }
    first += attribute->size();
  }

  error = make_error_code(errc::attribute_not_found);
  return {};
}


} // namespace __bits


__turner_end
