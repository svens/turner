#include <turner/__bits/message.hpp>
#include <turner/attribute.hpp>


__turner_begin


namespace __bits {


namespace {

constexpr size_t type_and_length = 2 * sizeof(uint16_t);

constexpr size_t round_up (size_t size, size_t padding_size) noexcept
{
  return type_and_length + ((size + padding_size - 1) & ~(padding_size - 1));
}

} // namespae


const any_attribute_t *find_attribute (
  const uint8_t *first, const uint8_t *last,
  uint16_t attribute_type,
  uint16_t integrity_attribute_type,
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
    else if (attribute->type() == integrity_attribute_type)
    {
      break;
    }
    first += round_up(attribute->length(), padding_size);
  }

  error = make_error_code(errc::attribute_not_found);
  return {};
}


size_t gather_unknown_comprehension_required_or_missing (
  const uint8_t *first,
  const uint8_t *last,
  const uint16_t attributes[],
  size_t attributes_count,
  uint16_t failed[],
  size_t failed_size,
  uint16_t integrity_attribute_type,
  size_t padding_size,
  std::error_code &error) noexcept
{
  size_t failed_count = 0;

  uint16_t message_attributes[1024];
  size_t message_attributes_count = 0;
  constexpr size_t message_attributes_size =
    sizeof(message_attributes)/sizeof(message_attributes[0]);

  //
  // unknown comprehension required
  // (also checks whether all attributes have enough payload data)
  //

  const uint16_t *attributes_first = attributes;
  const uint16_t *attributes_last = attributes + attributes_count;

  while (first + type_and_length <= last
    && failed_count < failed_size
    && message_attributes_count < message_attributes_size)
  {
    auto attribute = reinterpret_cast<const any_attribute_t *>(first);

    if (first + round_up(attribute->length(), padding_size) <= last)
    {
      auto attribute_type = attribute->type();
      if (attribute_type == integrity_attribute_type)
      {
        break;
      }
      message_attributes[message_attributes_count++] = attribute_type;

      if (attribute->is_comprehension_required())
      {
        auto it = std::find(attributes_first, attributes_last, attribute_type);
        if (it == attributes_last)
        {
          // comprehension required attribute but no plan to read it's value
          failed[failed_count++] = attribute_type;
        }
      }
    }
    else
    {
      error = make_error_code(errc::insufficient_payload_data);
      failed_count = 0;
      return {};
    }

    first += round_up(attribute->length(), padding_size);
  }

  if (failed_count)
  {
    error = make_error_code(errc::unknown_comprehension_required);
    return failed_count;
  }

  //
  // collect attributes planned to be read but not found in message
  //

  attributes_first = message_attributes;
  attributes_last = message_attributes + message_attributes_count;

  for (size_t i = 0;  i < attributes_count && failed_count < failed_size;  ++i)
  {
    auto it = std::find(attributes_first, attributes_last, attributes[i]);
    if (it == attributes_last)
    {
      failed[failed_count++] = attributes[i];
    }
  }

  if (failed_count)
  {
    error = make_error_code(errc::attribute_not_found);
  }

  return failed_count;
}


} // namespace __bits


__turner_end
