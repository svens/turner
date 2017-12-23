#include "bench.hpp"
#include <turner/stun/stun.hpp>
#include <turner/turn/turn.hpp>
#include <vector>


namespace {

template <typename Protocol>
constexpr const auto raw_message = false;


// STUN {{{1


using STUN = turner::stun::protocol_t;

template <>
const std::vector<uint8_t> raw_message<STUN> =
{
  // header
  0x00, 0x01, 0x00, 0x18,     // Type (Binding), Length
  0x21, 0x12, 0xa4, 0x42,     // Cookie
  0x00, 0x01, 0x02, 0x03,     // Transaction ID
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,

  // MESSAGE-INTEGRITY
  0x00, 0x08, 0x00, 0x14,     // Type, Length
  0xfb, 0xb7, 0x7f, 0xe5,     // 20B HMAC-SHA1
  0x34, 0xa2, 0x4b, 0xfa,
  0xa5, 0xf1, 0x52, 0x65,
  0x93, 0xfe, 0xf6, 0x59,
  0x9e, 0x9b, 0x64, 0x28,
};


// TURN {{{1


using TURN = turner::turn::protocol_t;

template <>
const std::vector<uint8_t> raw_message<TURN> =
{
  // header
  0x00, 0x03, 0x00, 0x18,     // Type (Allocation), Length
  0x21, 0x12, 0xa4, 0x42,     // Cookie
  0x00, 0x01, 0x02, 0x03,     // Transaction ID
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,

  // MESSAGE-INTEGRITY
  0x00, 0x08, 0x00, 0x14,     // Type, Length
  0x3f, 0x74, 0xd2, 0x2f,     // Type, Length
  0x2a, 0xed, 0xfe, 0xc3,     // 20B HMAC-SHA1
  0xe0, 0x82, 0x27, 0x29,
  0x6e, 0x3c, 0x22, 0x49,
  0x4a, 0x15, 0x2c, 0x23,
};


// }}}1


// parse_valid_with_error {{{1


template <typename Protocol>
void parse_valid_with_error (benchmark::State &state)
{
  auto data = raw_message<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }

  for (auto _: state)
  {
    msg = Protocol::parse(data, error);
    benchmark::DoNotOptimize(msg);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_valid_with_error, STUN);
BENCHMARK_TEMPLATE(parse_valid_with_error, TURN);


// parse_valid_with_exception {{{1


template <typename Protocol>
void parse_valid_with_exception (benchmark::State &state)
{
  auto data = raw_message<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }

  for (auto _: state)
  {
    msg = Protocol::parse(data);
    benchmark::DoNotOptimize(msg);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_valid_with_exception, STUN);
BENCHMARK_TEMPLATE(parse_valid_with_exception, TURN);


// parse_invalid_with_error {{{1


template <typename Protocol>
void parse_invalid_with_error (benchmark::State &state)
{
  auto data = raw_message<Protocol>;
  data[Protocol::traits_t::cookie_offset] ^= 1;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (!error)
  {
    state.SkipWithError("expected error");
    return;
  }

  for (auto _: state)
  {
    msg = Protocol::parse(data, error);
    benchmark::DoNotOptimize(msg);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_invalid_with_error, STUN);
BENCHMARK_TEMPLATE(parse_invalid_with_error, TURN);


// parse_invalid_with_exception {{{1


template <typename Protocol>
void parse_invalid_with_exception (benchmark::State &state)
{
  auto data = raw_message<Protocol>;
  data[Protocol::traits_t::cookie_offset] ^= 1;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (!error)
  {
    state.SkipWithError("expected error");
    return;
  }

  for (auto _: state)
  {
    try
    {
      msg = Protocol::parse(data);
      benchmark::DoNotOptimize(msg);
    }
    catch (...)
    {
      // ignore
    }
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_invalid_with_exception, STUN);
BENCHMARK_TEMPLATE(parse_invalid_with_exception, TURN);


// }}}1


} // namespace
