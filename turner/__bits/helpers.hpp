#pragma once

#include <turner/config.hpp>
#include <type_traits>
#include <utility>


__turner_begin


namespace __bits {


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


} // namespace __bits


__turner_end
