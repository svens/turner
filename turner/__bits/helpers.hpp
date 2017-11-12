#pragma once

#include <turner/config.hpp>
#include <memory>
#include <type_traits>


__turner_begin


namespace __bits {


// see https://tools.ietf.org/html/rfc5389#section-6
__turner_inline_var constexpr uint16_t
  method_mask =            0b110'0000'0000'0000,
  class_mask =             0b000'0001'0001'0000,
  indication_class =       0b000'0000'0001'0000,
  success_response_class = 0b000'0001'0000'0000,
  error_response_class =   0b000'0001'0001'0000;


// helper to detect if for type T, there is "operator>> (T, const char *&)"
template <typename T>
class has_name_getter_t
{
  template <typename U>
  static auto test (const char **x)
    -> decltype(std::declval<U&>() >> *x, std::true_type());

  template <typename U>
  static auto test (...)
    -> std::false_type;

public:

  static __turner_inline_var constexpr const bool value =
    decltype(test<T>(0))::value;
};


// instantiantion for has_name_getter_t
template <typename T>
__turner_inline_var constexpr bool has_name_getter_v =
  has_name_getter_t<T>::value;


// cast any iterator It to const byte pointer
template <typename It>
inline constexpr const uint8_t *to_cptr (It it) noexcept
{
  return reinterpret_cast<const uint8_t *>(std::addressof(*it));
}


// cast any iterator It to byte pointer
template <typename It>
inline constexpr uint8_t *to_ptr (It it) noexcept
{
  return reinterpret_cast<uint8_t *>(std::addressof(*it));
}


// silencer for msvc silly-warning C4996 (Checked Iterators)
template <typename It>
inline auto make_output_iterator (It first, It) noexcept
{
#if defined(_MSC_VER)
  return stdext::make_unchecked_array_iterator(first);
#else
  return first;
#endif
}


} // namespace __bits


__turner_end
