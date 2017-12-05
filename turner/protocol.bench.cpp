#include <turner/common.bench.hpp>
#include <turner/stun/stun.hpp>
#include <turner/turn/turn.hpp>


namespace {

template <typename Protocol>
constexpr const auto raw_message = false;


// STUN {{{1


using STUN = turner::stun::protocol_t;

template <>
constexpr const std::array<uint8_t, 44> raw_message<STUN> =
{{
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
}};


// TURN {{{1


using TURN = turner::turn::protocol_t;

template <>
constexpr const std::array<uint8_t, 44> raw_message<TURN> =
{{
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
}};


// }}}1


template <typename Protocol>
void parse (benchmark::State &state)
{
  // do initial parse for checking
  auto &data = raw_message<Protocol>;
  auto msg = Protocol::parse(data.begin(), data.end());

  std::error_code error;
  for (auto _: state)
  {
    msg = Protocol::parse(data.begin(), data.end(), error);
    benchmark::DoNotOptimize(msg);
  }
}

BENCHMARK_TEMPLATE(parse, STUN);
BENCHMARK_TEMPLATE(parse, TURN);


} // namespace
