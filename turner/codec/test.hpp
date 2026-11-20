#pragma once

/**
 * \file turner/codec/test.hpp
 * Test vectors and helpers
 * \internal
 */

#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/value.hpp>
#include <pal/net/ip/address_v4.hpp>
#include <pal/net/ip/port_type.hpp>
#include <pal/result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
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

inline constexpr auto binding_request_unknown_attributes = bytes(
	"00010028" "2112a442" "01020304" "05060708" "090a0b0c"
	"00240004" "6e0001ff"
	"80290008" "932ff9b1" "51263b36"
	"00060004" "6576746a"
	"00250000"
	"00240004" "6e0001ff"
);

inline constexpr auto binding_request_many_unknown_attributes = bytes(
	"00010014" "2112a442" "01020304" "05060708" "090a0b0c"
	"00310000"
	"00320000"
	"00330000"
	"00340000"
	"00350000"
);

inline constexpr auto binding_error = bytes(
	"01110014" "2112a442" "01020304" "05060708" "090a0b0c"
	"00090010" "00000401" "556e6175" "74686f72" "697a6564"
);

inline constexpr auto binding_request_fingerprint = bytes(
	"00010008" "2112a442" "01020304" "05060708" "090a0b0c"
	"80280004" "5b20f9cc"
);

inline constexpr auto binding_request_software_fingerprint = bytes(
	"00010014" "2112a442" "01020304" "05060708" "090a0b0c"
	"80220005" "68656c6c" "6f000000"
	"80280004" "0a8b9190"
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

inline constexpr auto set_active_destination_request_0x8028 = bytes(
	"00060010" "2112a442" "01020304" "05060708" "090a0b0c"
	"000f0004" "72c64bc6"
	"80280004" "deadbeef"
);

} // namespace msturn_message

namespace integrity_message
{

inline constexpr std::string_view username = "user";
inline constexpr std::string_view realm = "realm";
inline constexpr std::string_view quoted_realm = "\"realm\"";
inline constexpr std::string_view nonce = "nonce";
inline constexpr std::string_view password = "pass";

inline constexpr auto stun_sha1 = bytes(
	"00010020" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "00080014" "0f9187c6" "a30f64fe" "dd9f3783" "46be6e2f"
	"d8b0fd74"
);

inline constexpr auto stun_sha256 = bytes(
	"0001002c" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "001c0020" "28a5a925" "10439ca6" "131e20f2" "1ed8ff29"
	"7e139904" "c48e2a73" "014b8749" "e984d1be"
);

inline constexpr auto stun_md5_key_sha256 = bytes(
	"0001002c" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "001c0020" "bec867be" "c4eac54e" "db695094" "8664b0da"
	"782548f6" "b0f650ca" "34acea15" "6262a539"
);

inline constexpr auto stun_sha1_fingerprint = bytes(
	"00010028" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "00080014" "0f9187c6" "a30f64fe" "dd9f3783" "46be6e2f"
	"d8b0fd74" "80280004" "25d72cbf"
);

inline constexpr auto stun_sha256_fingerprint = bytes(
	"00010034" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "001c0020" "28a5a925" "10439ca6" "131e20f2" "1ed8ff29"
	"7e139904" "c48e2a73" "014b8749" "e984d1be" "80280004" "17f64d92"
);

inline constexpr auto stun_sha1_sha256 = bytes(
	"00010044" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "00080014" "0f9187c6" "a30f64fe" "dd9f3783" "46be6e2f"
	"d8b0fd74" "001c0020" "fd12618d" "c43388a1" "6dd453c3" "bf9c4f49"
	"13c8e312" "9837b05a" "d9963811" "7a06c881"
);

inline constexpr auto stun_sha1_software = bytes(
	"0001002c" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "00080014" "0f9187c6" "a30f64fe" "dd9f3783" "46be6e2f"
	"d8b0fd74" "80220005" "68656c6c" "6f000000"
);

inline constexpr auto stun_sha1_twice = bytes(
	"00010038" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "00080014" "0f9187c6" "a30f64fe" "dd9f3783" "46be6e2f"
	"d8b0fd74" "00080014" "00000000" "00000000" "00000000" "00000000"
	"00000000"
);

inline constexpr auto stun_empty_realm = bytes(
	"00010020" "2112a442" "01020304" "05060708" "090a0b0c" "00060004"
	"75736572" "00080014" "c97297f3" "c08648a5" "4d31f685" "02534374"
	"0433f663"
);

inline constexpr auto turn_allocate_sha1 = bytes(
	"00030020" "2112a442" "01020304" "05060708" "090a0b0c" "00190004"
	"11000000" "00080014" "719cc079" "2ae2ff1c" "b63f98fd" "2ec2d1a6"
	"bd50091f"
);

inline constexpr auto msturn_sha1 = bytes(
	"0003003c" "2112a442" "01020304" "05060708" "090a0b0c" "000f0004"
	"72c64bc6" "00060004" "75736572" "00150007" "22726561" "6c6d2200"
	"1400056e" "6f6e6365" "00080014" "29d22d5f" "0a3f2a55" "bb9046cb"
	"5b2092d7" "d76cb25e"
);

inline constexpr auto msturn_sha256 = bytes(
	"00030048" "2112a442" "01020304" "05060708" "090a0b0c" "000f0004"
	"72c64bc6" "00060004" "75736572" "00150007" "22726561" "6c6d2200"
	"1400056e" "6f6e6365" "00080020" "c987d5ef" "152342e9" "ced2eb31"
	"fe44df65" "38949918" "9269914b" "afab4918" "c0486d62"
);

inline constexpr auto msturn_covers_64 = bytes(
	"00030044" "2112a442" "01020304" "05060708" "090a0b0c" "000f0004"
	"72c64bc6" "00060020" "75757575" "75757575" "75757575" "75757575"
	"75757575" "75757575" "75757575" "75757575" "00080014" "b136c4d1"
	"fd2b548a" "2c4b88b5" "2339ff5a" "73a28e03"
);

inline constexpr auto msturn_covers_65 = bytes(
	"00030045" "2112a442" "01020304" "05060708" "090a0b0c" "000f0004"
	"72c64bc6" "00060021" "75757575" "75757575" "75757575" "75757575"
	"75757575" "75757575" "75757575" "75757575" "75000800" "14eba692"
	"c5abfeb1" "9efdde74" "df591680" "90b1dedd" "58"
);

inline constexpr auto msturn_long_nonce = bytes(
	"00030071" "2112a442" "01020304" "05060708" "090a0b0c" "000f0004"
	"72c64bc6" "00140041" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e"
	"6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e"
	"6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e" "6e6e6e6e"
	"6e000800" "2021b3e5" "9f6dea7d" "b9a59833" "a0e0f13b" "d6fd4e71"
	"9a0f6fae" "e815b786" "08584884" "63"
);

} // namespace integrity_message

inline constexpr std::byte filler{0xff};
inline constexpr std::byte garbage{0xa5};

inline constexpr std::array<std::byte, 12> stun_id = bytes("0102030405060708090a0b0c");
inline constexpr std::array<std::byte, 16> msturn_id = bytes("2112a442" "0102030405060708090a0b0c");

inline constexpr auto test_ipv4 = pal::net::ip::make_address_v4(pal::net::ip::address_v4::bytes_type{192, 0, 2, 1});
inline constexpr turner::transport_address peer_address{test_ipv4, pal::net::ip::port_type{32853}};
inline constexpr turner::transport_address server_address{test_ipv4, pal::net::ip::port_type{3478}};

inline std::vector<std::byte> make_buffer (size_t size = 256)
{
	return std::vector<std::byte>(size, filler);
}

inline std::vector<std::byte> buffer_of (std::span<const std::byte> bytes, size_t trailing = 0)
{
	std::vector<std::byte> buffer{bytes.begin(), bytes.end()};
	buffer.resize(buffer.size() + trailing, garbage);
	return buffer;
}

template <size_t Size>
std::array<std::byte, Size> make_array () noexcept
{
	std::array<std::byte, Size> buffer{};
	buffer.fill(filler);
	return buffer;
}

inline std::span<const std::byte> as_bytes (std::string_view text) noexcept
{
	return std::as_bytes(std::span{text});
}

inline pal::unexpected failed (turner::errc error) noexcept
{
	return turner::make_unexpected(error);
}

inline std::string hex (std::span<const std::byte> bytes)
{
	constexpr std::string_view digits = "0123456789abcdef";
	std::string result;
	for (const auto byte: bytes)
	{
		result += digits[std::to_integer<size_t>(byte) >> 4];
		result += digits[std::to_integer<size_t>(byte) & 0xf];
	}
	return result;
}

template <std::same_as<pal::result<std::span<const std::byte>>> Message>
std::string hex (const Message &message)
{
	return message ? hex(*message) : message.error().message();
}

inline turner::integrity_credentials integrity_credentials () noexcept
{
	namespace m = integrity_message;
	return {.username = m::username, .realm = m::realm, .nonce = m::nonce, .password = m::password};
}

template <typename Protocol>
typename Protocol::integrity_key sha1_key ()
{
	auto key = Protocol::sha1_key(integrity_credentials());
	REQUIRE(key);
	return std::move(*key);
}

template <typename Protocol>
typename Protocol::integrity_key sha256_key ()
{
	auto key = Protocol::sha256_key(integrity_credentials());
	REQUIRE(key);
	return std::move(*key);
}

template <typename Protocol, auto Entity>
turner::message_reader<Entity> parse_in (std::span<std::byte> buffer, std::span<const std::byte> message)
{
	std::ranges::copy(message, buffer.begin());
	const auto parsed = Protocol::parse(buffer, message.size());
	REQUIRE(parsed);
	const auto reader = parsed->as(Entity);
	REQUIRE(reader);
	return *reader;
}

template <typename Protocol, auto Entity>
turner::message_reader<Entity> parse_as (std::span<const std::byte> message, std::vector<std::byte> &buffer)
{
	buffer.resize(message.size());
	return parse_in<Protocol, Entity>(buffer, message);
}

} // namespace turner_test
