#pragma once

#include <benchmark/benchmark.h>
#include <turner/msturn/msturn.hpp>
#include <turner/stun/stun.hpp>
#include <turner/turn/turn.hpp>
#include <vector>


namespace turner_bench {


template <typename Protocol>
inline constexpr const auto raw_message = false;

template <typename Protocol>
inline constexpr const auto message_type = false;

template <typename Protocol>
inline constexpr const auto another_message_type = false;


// MSTURN {{{1


using MSTURN = turner::msturn::protocol_t;

template <>
inline const std::vector<uint8_t> raw_message<MSTURN> =
{
  // header
  0x00, 0x03, 0x00, 0x20,     // Type (Allocate), Length
  0x00, 0x01, 0x02, 0x03,     // Transaction ID
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,
  0x0c, 0x0d, 0x0e, 0x0f,

  // Magic Cookie
  0x00, 0x0f, 0x00, 0x04,
  0x72, 0xc6, 0x4b, 0xc6,

  // Message Integrity
  0x00, 0x08, 0x00, 0x14,     // Type, Length
  0xf4, 0xaf, 0x03, 0xc6,     // 20B HMAC-SHA1
  0x21, 0x6f, 0x19, 0x76,
  0x65, 0x9f, 0x87, 0x21,
  0x18, 0x7c, 0xff, 0xc5,
  0x9e, 0xb7, 0x32, 0x2a,
};

template <>
inline constexpr const auto message_type<MSTURN> = turner::msturn::allocate;

template <>
inline constexpr const auto another_message_type<MSTURN> = turner::msturn::allocate_success;


// STUN {{{1


using STUN = turner::stun::protocol_t;

template <>
inline const std::vector<uint8_t> raw_message<STUN> =
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

template <>
inline constexpr const auto message_type<STUN> = turner::stun::binding;

template <>
inline constexpr const auto another_message_type<STUN> = turner::stun::binding_success;


// TURN {{{1


using TURN = turner::turn::protocol_t;

template <>
inline const std::vector<uint8_t> raw_message<TURN> =
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

template <>
inline constexpr const auto message_type<TURN> = turner::turn::allocation;

template <>
inline constexpr const auto another_message_type<TURN> = turner::turn::allocation_success;


// }}}1


} // namespace turner_bench
