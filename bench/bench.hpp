#pragma once

#include <benchmark/benchmark.h>
#include <turner/msturn/msturn.hpp>
#include <turner/stun/stun.hpp>
#include <turner/turn/turn.hpp>
#include <vector>


namespace turner_bench {


template <typename Protocol>
inline constexpr const auto wire_data = false;

template <typename Protocol>
inline constexpr const auto message_type = false;

template <typename Protocol>
inline constexpr const auto another_message_type = false;

template <typename Protocol>
inline constexpr const auto attribute_type = false;

template <typename Protocol>
inline constexpr const auto another_attribute_type = false;


// MSTURN {{{1


using MSTURN = turner::msturn::protocol_t;

template <>
inline const std::vector<uint8_t> wire_data<MSTURN> =
{
  // Allocate
  0x00, 0x03, 0x00, 0x28,
  0x59, 0xcd, 0x65, 0x23,
  0x7c, 0x05, 0xea, 0xfa,
  0x94, 0xde, 0x75, 0x7f,
  0xc7, 0x0a, 0x37, 0xcb,

  // Magic Cookie
  0x00, 0x0f, 0x00, 0x04,
  0x72, 0xc6, 0x4b, 0xc6,

  // Lifetime (600)
  0x00, 0x0d, 0x00, 0x04,
  0x00, 0x00, 0x02, 0x58,

  // Message Integrity
  0x00, 0x08, 0x00, 0x14,
  0xaa, 0xbe, 0x88, 0x0e,
  0x8d, 0x71, 0x97, 0x7a,
  0x56, 0x3a, 0x27, 0x70,
  0xc3, 0xda, 0x3a, 0xcf,
  0xb8, 0x1e, 0xbe, 0x14,
};

template <>
inline constexpr const auto message_type<MSTURN> =
  turner::msturn::allocate;

template <>
inline constexpr const auto another_message_type<MSTURN> =
  turner::msturn::allocate_success;

template <>
inline constexpr const auto attribute_type<MSTURN> =
  turner::msturn::lifetime;

template <>
inline constexpr const auto another_attribute_type<MSTURN> =
  turner::msturn::username;

inline auto integrity_calculator (MSTURN)
{
  return turner::msturn::make_integrity_calculator("realm", "user", "pass");
}


// STUN {{{1


using STUN = turner::stun::protocol_t;

template <>
inline const std::vector<uint8_t> wire_data<STUN> =
{
  // Binding
  0x00, 0x01, 0x00, 0x20,
  0x21, 0x12, 0xa4, 0x42,
  0x84, 0xa1, 0x21, 0x2e,
  0x73, 0xab, 0xfd, 0x5b,
  0xf7, 0x19, 0x35, 0xf4,

  // LIFETIME (600)
  0x00, 0x0d, 0x00, 0x04,
  0x00, 0x00, 0x02, 0x58,

  // MESSAGE-INTEGRITY
  0x00, 0x08, 0x00, 0x14,
  0x6e, 0x86, 0x3f, 0x4e,
  0x70, 0x48, 0x3c, 0x2b,
  0xb5, 0xe6, 0xf0, 0x2b,
  0xc7, 0x59, 0x2c, 0x37,
  0x11, 0x0e, 0x16, 0x25,
};

template <>
inline constexpr const auto message_type<STUN> =
  turner::stun::binding;

template <>
inline constexpr const auto another_message_type<STUN> =
  turner::stun::binding_success;

template <>
inline constexpr const auto attribute_type<STUN> =
  turner::stun::protocol_t::reuse_attribute_t<turner::turn::lifetime_t>{};

template <>
inline constexpr const auto another_attribute_type<STUN> =
  turner::stun::username;

inline auto integrity_calculator (STUN)
{
  return turner::stun::make_integrity_calculator("realm", "user", "pass");
}


// TURN {{{1


using TURN = turner::turn::protocol_t;

template <>
inline const std::vector<uint8_t> wire_data<TURN> =
{
  // Allocation
  0x00, 0x03, 0x00, 0x20,
  0x21, 0x12, 0xa4, 0x42,
  0x32, 0x21, 0x4e, 0x24,
  0x96, 0xf6, 0xd9, 0x32,
  0x6b, 0x1f, 0xa0, 0x8a,

  // LIFETIME (600)
  0x00, 0x0d, 0x00, 0x04,
  0x00, 0x00, 0x02, 0x58,

  // MESSAGE-INTEGRITY
  0x00, 0x08, 0x00, 0x14,
  0xb6, 0x5a, 0x70, 0xee,
  0x88, 0x92, 0xa5, 0x74,
  0x2f, 0xc7, 0x6b, 0xb7,
  0xc2, 0x79, 0x3e, 0x3a,
  0x0b, 0x7e, 0xfd, 0x7e,

};

template <>
inline constexpr const auto message_type<TURN> =
  turner::turn::allocation;

template <>
inline constexpr const auto another_message_type<TURN> =
  turner::turn::allocation_success;

template <>
inline constexpr const auto attribute_type<TURN> =
  turner::turn::lifetime;

template <>
inline constexpr const auto another_attribute_type<TURN> =
  turner::turn::username;

inline auto integrity_calculator (TURN)
{
  return turner::turn::make_integrity_calculator("realm", "user", "pass");
}


// }}}1


template <typename Protocol, size_t N>
auto wire_data_with_payload (Protocol, const char (&data)[N])
{
  // get raw data, replace payload with data
  auto raw = wire_data<Protocol>;
  raw.resize(Protocol::header_and_cookie_size());
  raw.insert(raw.end(), data, data + N - 1);
  reinterpret_cast<uint16_t *>(&raw[0])[1] = sal::native_to_network_byte_order(
    static_cast<uint16_t>(N - 1 + Protocol::min_payload_length())
  );
  return raw;
}


} // namespace turner_bench
