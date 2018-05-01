#pragma once
#include <chrono>
#include <system_error>
#include <gmock/gmock.h>


namespace turner_test {


#if __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif


template <bool IsStream>
struct transport_mock_t
{
  static constexpr bool is_stream = IsStream;

  MOCK_METHOD2(wait_receive_for,
    bool(std::chrono::milliseconds, std::error_code &)
  );
  MOCK_METHOD3(receive,
    size_t(uint8_t *, uint8_t *, std::error_code &)
  );
  MOCK_METHOD3(send,
    bool(const uint8_t *, const uint8_t *, std::error_code &)
  );
};


#if __GNUC__
  #pragma GCC diagnostic pop
#endif


} // namespace turner_test
