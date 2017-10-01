#include <turner/message.hpp>
#include <sal/config.hpp>

#if __sal_os_windows
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32")
#endif


__turner_begin


message_type_t message_base_t::type () const noexcept
{
  sal_assert(is_valid());
  return ntohs(reinterpret_cast<const uint16_t *>(begin_)[0]);
}


uint16_t message_base_t::length () const noexcept
{
  sal_assert(is_valid());
  return ntohs(reinterpret_cast<const uint16_t *>(begin_)[1]);
}


__turner_end
