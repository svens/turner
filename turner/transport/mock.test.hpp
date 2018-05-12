#pragma once
#include <chrono>
#include <iterator>
#include <limits>
#include <vector>
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


struct transport_feed_t
{
  std::vector<uint8_t> data{};
  std::vector<uint8_t>::const_iterator ptr{};
  size_t chunk_size = (std::numeric_limits<size_t>::max)();


  transport_feed_t () = default;


  template <typename Data>
  transport_feed_t (const Data &content)
    : data(std::cbegin(content), std::cend(content))
    , ptr(std::cbegin(data))
  { }


  size_t receive (uint8_t *first, uint8_t *last, std::error_code &)
  {
    size_t size = (std::min)(last - first, data.end() - ptr);
    if (chunk_size && chunk_size < size)
    {
      size = chunk_size;
    }

    std::uninitialized_copy_n(ptr, size,
      sal::__bits::make_output_iterator(first, last)
    );
    ptr += size;

    return size;
  }


  bool send (const uint8_t *first, const uint8_t *last, std::error_code &)
  {
    data.assign(first, last);
    ptr = data.cbegin();
    return true;
  }
};


} // namespace turner_test


//
// Intentionally kept out of namespace to beautify gtest names
//


struct datagram
  : public ::testing::NiceMock<::turner_test::transport_mock_t<false>>
{};


struct stream
  : public ::testing::NiceMock<::turner_test::transport_mock_t<true>>
{};


using transport_types = ::testing::Types<datagram, stream>;
