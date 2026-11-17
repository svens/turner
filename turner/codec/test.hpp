#pragma once

/**
 * \file turner/codec/test.hpp
 * Test vectors
 * \internal
 */

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace turner_test
{

constexpr unsigned hex_digit (char c)
{
	if ('0' <= c && c <= '9')
	{
		return static_cast<unsigned>(c - '0');
	}
	if ('a' <= c && c <= 'f')
	{
		return static_cast<unsigned>(c - 'a' + 10);
	}
	throw std::invalid_argument{"invalid hex digit"};
}

constexpr std::byte hex_byte (char high, char low)
{
	return static_cast<std::byte>((hex_digit(high) << 4) | hex_digit(low));
}

template <size_t N>
constexpr std::array<std::byte, (N - 1) / 2> bytes (const char (&hex)[N])
{
	static_assert(N % 2 == 1, "odd number of hex digits");

	std::array<std::byte, (N - 1) / 2> result{};
	for (size_t i = 0; i < result.size(); ++i)
	{
		result[i] = hex_byte(hex[2 * i], hex[2 * i + 1]);
	}
	return result;
}

inline std::vector<std::byte> to_bytes (std::string_view hex)
{
	if (hex.size() % 2 != 0)
	{
		throw std::invalid_argument{"odd number of hex digits"};
	}

	std::vector<std::byte> result(hex.size() / 2);
	for (size_t i = 0; i < result.size(); ++i)
	{
		result[i] = hex_byte(hex[2 * i], hex[2 * i + 1]);
	}
	return result;
}

namespace rfc5769
{

inline constexpr auto request = bytes(
	"00010058" "2112a442" "b7e7a701" "bc34d686" "fa87dfae"
	"80220010" "5354554e" "20746573" "7420636c" "69656e74"
	"00240004" "6e0001ff"
	"80290008" "932ff9b1" "51263b36"
	"00060009" "6576746a" "3a683676" "59202020"
	"00080014" "9aeaa70c" "bfd8cb56" "781ef2b5" "b2d3f249" "c1b571a2"
	"80280004" "e57a3bcf"
);

inline constexpr auto ipv4_response = bytes(
	"0101003c" "2112a442" "b7e7a701" "bc34d686" "fa87dfae"
	"8022000b" "74657374" "20766563" "746f7220"
	"00200008" "0001a147" "e112a643"
	"00080014" "2b91f599" "fd9e90c3" "8c7489f9" "2af9ba53" "f06be7d7"
	"80280004" "c07d4c96"
);

inline constexpr auto ipv6_response = bytes(
	"01010048" "2112a442" "b7e7a701" "bc34d686" "fa87dfae"
	"8022000b" "74657374" "20766563" "746f7220"
	"00200014" "0002a147" "0113a9fa" "a5d3f179" "bc25f4b5" "bed2b9d9"
	"00080014" "a382954e" "4be67bf1" "1784c97c" "8292c275" "bfe3ed41"
	"80280004" "c8fb0b4c"
);

inline constexpr auto long_term_request = bytes(
	"00010060" "2112a442" "78ad3433" "c6ad72c0" "29da412e"
	"00060012" "e3839ee3" "8388e383" "aae38383" "e382afe3" "82b90000"
	"0015001c" "662f2f34" "39396b39" "35346436" "4f4c3334" "6f4c3946" "53547679" "36347341"
	"0014000b" "6578616d" "706c652e" "6f726700"
	"00080014" "f6702465" "6dd64a3e" "02b8e071" "2e85c9a2" "8ca89666"
);

} // namespace rfc5769

namespace stun_message
{

inline constexpr auto binding_indication = bytes(
	"00110000" "2112a442" "01020304" "05060708" "090a0b0c"
);

inline constexpr auto binding_error = bytes(
	"01110014" "2112a442" "01020304" "05060708" "090a0b0c"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
);

} // namespace stun_message

namespace turn_message
{

inline constexpr auto allocate_request = bytes(
	"00030008" "2112a442" "01020304" "05060708" "090a0b0c"
	"00190004" "11000000"
);

inline constexpr auto allocate_request_dont_fragment = bytes(
	"0003000c" "2112a442" "01020304" "05060708" "090a0b0c"
	"001a0000"
	"00190004" "11000000"
);

inline constexpr auto allocate_success = bytes(
	"01030020" "2112a442" "01020304" "05060708" "090a0b0c"
	"00160008" "0001a147" "e112a643"
	"000d0004" "00000258"
	"00200008" "0001a147" "e112a643"
);

inline constexpr auto allocate_error = bytes(
	"01130030" "2112a442" "01020304" "05060708" "090a0b0c"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
	"0014000b" "6578616d" "706c652e" "6f726700"
	"00150005" "6e6f6e63" "65000000"
);

inline constexpr auto refresh_request = bytes(
	"00040008" "2112a442" "01020304" "05060708" "090a0b0c"
	"000d0004" "00000000"
);

inline constexpr auto refresh_request_repeated_lifetime = bytes(
	"00040028" "2112a442" "01020304" "05060708" "090a0b0c"
	"000d0004" "0000000a"
	"80220004" "74657374"
	"000d0004" "00000014"
	"000d0002" "00010000"
	"00130004" "01020304"
);

inline constexpr auto refresh_request_malformed_lifetime_first = bytes(
	"00040010" "2112a442" "01020304" "05060708" "090a0b0c"
	"000d0002" "00010000"
	"000d0004" "0000000a"
);

inline constexpr auto refresh_success = bytes(
	"01040000" "2112a442" "01020304" "05060708" "090a0b0c"
);

inline constexpr auto refresh_error = bytes(
	"01140014" "2112a442" "01020304" "05060708" "090a0b0c"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
);

inline constexpr auto send_indication = bytes(
	"00160018" "2112a442" "01020304" "05060708" "090a0b0c"
	"00120008" "0001a147" "e112a643"
	"00130005" "68656c6c" "6f000000"
);

inline constexpr auto data_indication = bytes(
	"00170018" "2112a442" "01020304" "05060708" "090a0b0c"
	"00120008" "0001a147" "e112a643"
	"00130005" "68656c6c" "6f000000"
);

inline constexpr auto create_permission_request = bytes(
	"0008000c" "2112a442" "01020304" "05060708" "090a0b0c"
	"00120008" "0001a147" "e112a643"
);

inline constexpr auto create_permission_success = bytes(
	"01080000" "2112a442" "01020304" "05060708" "090a0b0c"
);

inline constexpr auto create_permission_error = bytes(
	"01180014" "2112a442" "01020304" "05060708" "090a0b0c"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
);

inline constexpr auto channel_bind_request = bytes(
	"00090014" "2112a442" "01020304" "05060708" "090a0b0c"
	"000c0004" "40000000"
	"00120008" "0001a147" "e112a643"
);

inline constexpr auto channel_bind_success = bytes(
	"01090000" "2112a442" "01020304" "05060708" "090a0b0c"
);

inline constexpr auto channel_bind_error = bytes(
	"01190014" "2112a442" "01020304" "05060708" "090a0b0c"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
);

} // namespace turn_message

namespace msturn_message
{

inline constexpr auto allocate_request = bytes(
	"00030021" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"80080004" "00000004"
	"80550004" "00010000"
	"00060005" "616c6963" "65"
);

inline constexpr auto allocate_request_empty_attributes = bytes(
	"00030018" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"00060000"
	"80080004" "00000004"
	"00150000"
);

inline constexpr auto allocate_success = bytes(
	"01030025" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"00010008" "00010d96" "c0000201"
	"000d0004" "00000258"
	"00060005" "616c6963" "65"
);

inline constexpr auto allocate_success_ipv6 = bytes(
	"01030020" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"80900014" "00020d96" "20010db8" "00000000" "00000000" "00000001"
);

inline constexpr auto allocate_success_ipv4_in_ipv6_only = bytes(
	"01030014" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"80900008" "00010d96" "c0000201"
);

inline constexpr auto allocate_error = bytes(
	"0113003c" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"80080004" "00000004"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
	"00140005" "6e6f6e63" "65"
	"0015000b" "6578616d" "706c652e" "6f7267"
);

inline constexpr auto send_request = bytes(
	"0004002c" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"80080004" "00000004"
	"00130003" "616263"
	"00110008" "00010d96" "c0000201"
	"00060005" "616c6963" "65"
);

inline constexpr auto data_indication = bytes(
	"0115001b" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"00120008" "00010d96" "c0000201"
	"00130003" "616263"
);

inline constexpr auto set_active_destination_request = bytes(
	"00060014" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"00110008" "00010d96" "c0000201"
);

inline constexpr auto set_active_destination_success = bytes(
	"01060008" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
);

inline constexpr auto set_active_destination_error = bytes(
	"0116001c" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
);

} // namespace msturn_message

} // namespace turner_test
