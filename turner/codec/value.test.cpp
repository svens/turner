#include <turner/codec/test.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol_error.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <pal/net/ip/address_v4.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <pal/net/ip/port_type.hpp>
#include <pal/net/ip/udp.hpp>
#include <pal/result.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace
{

using namespace std::chrono_literals;
using namespace std::string_view_literals;

struct valid_value
{
	using native_type = int;
	static constexpr size_t min_size = 1;
	static constexpr size_t max_size = 1;
	static size_t size (const int &) noexcept;
	static pal::result<int> read (std::span<const std::byte>, std::span<const std::byte>) noexcept;
	static void write (std::span<const std::byte>, std::span<std::byte>, const int &) noexcept;
};

struct without_min_size: valid_value
{
	static void min_size () = delete;
};

struct without_max_size: valid_value
{
	static void max_size () = delete;
};

struct with_throwing_size: valid_value
{
	static size_t size (const int &);
};

struct with_other_size_result: valid_value
{
	static int size (const int &) noexcept;
};

struct with_throwing_read: valid_value
{
	static pal::result<int> read (std::span<const std::byte>, std::span<const std::byte>);
};

struct with_other_read_result: valid_value
{
	static int read (std::span<const std::byte>, std::span<const std::byte>) noexcept;
};

struct with_throwing_write: valid_value
{
	static void write (std::span<const std::byte>, std::span<std::byte>, const int &);
};

struct with_other_write_result: valid_value
{
	static bool write (std::span<const std::byte>, std::span<std::byte>, const int &) noexcept;
};

struct without_write: valid_value
{
	static void write () = delete;
};

static_assert(turner::attribute_value<valid_value>);
static_assert(!turner::attribute_value<without_min_size>);
static_assert(!turner::attribute_value<without_max_size>);
static_assert(!turner::attribute_value<with_throwing_size>);
static_assert(!turner::attribute_value<with_other_size_result>);
static_assert(!turner::attribute_value<with_throwing_read>);
static_assert(!turner::attribute_value<with_other_read_result>);
static_assert(!turner::attribute_value<with_throwing_write>);
static_assert(!turner::attribute_value<with_other_write_result>);
static_assert(!turner::attribute_value<without_write>);
static_assert(!turner::attribute_value<int>);
static_assert(!turner::attribute_value<std::string_view>);

template <typename Value>
struct sample
{
	std::string_view wire;
	typename Value::native_type native;
};

struct uint16
{
	using value_type = turner::integer_value<uint16_t>;
	using native_type = uint16_t;
	static constexpr size_t min_size = 2;
	static constexpr size_t max_size = 2;

	static constexpr sample<value_type> samples[] =
	{
		{"0000", 0},
		{"0102", 0x0102},
		{"ffff", 0xffff},
	};
};

struct uint32
{
	using value_type = turner::uint32_value;
	using native_type = uint32_t;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"00000000", 0},
		{"01020304", 0x01020304},
		{"ffffffff", 0xffffffff},
	};
};

struct seconds
{
	using value_type = turner::seconds_value;
	using native_type = std::chrono::seconds;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"00000000", 0s},
		{"00000258", 600s},
		{"ffffffff", std::chrono::seconds{0xffff'ffff}},
	};
};

struct string
{
	using value_type = turner::string_value<>;
	using native_type = std::string_view;
	static constexpr size_t min_size = 0;
	static constexpr size_t max_size = 0xffff;

	static constexpr sample<value_type> samples[] =
	{
		{"", ""},
		{"616c696365", "alice"},
		{"610062", "a\0b"sv},
	};
};

struct limited_string
{
	using value_type = turner::string_value<128>;
	using native_type = std::string_view;
	static constexpr size_t min_size = 0;
	static constexpr size_t max_size = 128;

	static constexpr sample<value_type> samples[] =
	{
		{"", ""},
		{"6e6f6e6365", "nonce"},
	};
};

struct bytes
{
	using value_type = turner::bytes_value<>;
	using native_type = std::span<const std::byte>;
	static constexpr size_t min_size = 0;
	static constexpr size_t max_size = 0xffff;

	static constexpr auto three = turner_test::bytes("010203");

	static constexpr sample<value_type> samples[] =
	{
		{"", {}},
		{"010203", three},
	};
};

struct fixed_bytes
{
	using value_type = turner::bytes_value<8>;
	using native_type = std::span<const std::byte, 8>;
	static constexpr size_t min_size = 8;
	static constexpr size_t max_size = 8;

	static constexpr auto eight = turner_test::bytes("0102030405060708");

	static constexpr sample<value_type> samples[] =
	{
		{"0102030405060708", native_type{eight}},
	};
};

struct flag
{
	using value_type = turner::flag_value;
	using native_type = std::true_type;
	static constexpr size_t min_size = 0;
	static constexpr size_t max_size = 0;

	static constexpr sample<value_type> samples[] =
	{
		{"", {}},
	};
};

struct address_family
{
	using value_type = turner::address_family_value;
	using native_type = turner::address_family;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"01000000", turner::address_family::v4},
		{"02000000", turner::address_family::v6},
	};
};

struct transport_protocol
{
	using value_type = turner::transport_protocol_value;
	using native_type = turner::transport_protocol;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"06000000", turner::transport_protocol::tcp},
		{"11000000", turner::transport_protocol::udp},
	};
};

constexpr pal::net::ip::port_type stun_port{3478};
constexpr auto ipv4 = pal::net::ip::make_address_v4(pal::net::ip::address_v4::bytes_type{192, 0, 2, 1});
constexpr auto ipv6 = pal::net::ip::make_address_v6(pal::net::ip::address_v6::bytes_type{
	0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
});

struct endpoint
{
	using value_type = turner::endpoint_value;
	using native_type = turner::transport_address;
	static constexpr size_t min_size = 8;
	static constexpr size_t max_size = 20;

	static constexpr sample<value_type> samples[] =
	{
		{"00010d96" "c0000201", {ipv4, stun_port}},
		{"00020d96" "20010db8" "00000000" "00000000" "00000001", {ipv6, stun_port}},
	};
};

struct xor_endpoint
{
	using value_type = turner::xor_endpoint_value;
	using native_type = turner::transport_address;
	static constexpr size_t min_size = 8;
	static constexpr size_t max_size = 20;

	static constexpr std::string_view message = "0101003c" "2112a442" "b7e7a701" "bc34d686" "fa87dfae";

	static constexpr pal::net::ip::port_type rfc5769_port{32853};
	static constexpr auto rfc5769_ipv6 = pal::net::ip::make_address_v6(pal::net::ip::address_v6::bytes_type{
		0x20, 0x01, 0x0d, 0xb8, 0x12, 0x34, 0x56, 0x78, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	});

	static constexpr sample<value_type> samples[] =
	{
		{"0001a147" "e112a643", {ipv4, rfc5769_port}},
		{"0002a147" "0113a9fa" "a5d3f179" "bc25f4b5" "bed2b9d9", {rfc5769_ipv6, rfc5769_port}},
	};
};

struct error_code
{
	using value_type = turner::error_code_value<>;
	using native_type = turner::protocol_error;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4 + 763;

	static constexpr sample<value_type> samples[] =
	{
		{"00000401" "556e6175" "74686f72" "697a6564", {turner::protocol_errc::unauthorized, "Unauthorized"}},
		{"00000300", {turner::protocol_errc::try_alternate, ""}},
		{"00000663" "78", {static_cast<turner::protocol_errc>(699), "x"}},
	};
};

struct unlimited_error_code
{
	using value_type = turner::error_code_value<0xffff - 4>;
	using native_type = turner::protocol_error;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 0xffff;

	static constexpr sample<value_type> samples[] =
	{
		{"00000426" "5374616c" "65204e6f" "6e6365", {turner::protocol_errc::stale_nonce, "Stale Nonce"}},
	};
};

struct unknown_attributes
{
	using value_type = turner::unknown_attributes_value;
	using native_type = turner::attribute_type_list;
	static constexpr size_t min_size = 2;
	static constexpr size_t max_size = 0xfffe;

	static constexpr sample<value_type> samples[] =
	{
		{"000a", {{0x000a}}},
		{"000a" "8000" "0001" "c001", {{0x000a, 0x8000, 0x0001, 0xc001}}},
	};
};

struct ipv6_endpoint
{
	using value_type = turner::msturn::ipv6_endpoint_value;
	using native_type = turner::transport_address;
	static constexpr size_t min_size = 20;
	static constexpr size_t max_size = 20;

	static constexpr sample<value_type> samples[] =
	{
		{"00020d96" "20010db8" "00000000" "00000000" "00000001", {ipv6, stun_port}},
	};
};

struct password_algorithm
{
	using value_type = turner::password_algorithm_value;
	using native_type = turner::password_algorithm;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"00010000", turner::password_algorithm::md5},
		{"00020000", turner::password_algorithm::sha256},
	};
};

struct password_algorithms
{
	using value_type = turner::password_algorithms_value;
	using native_type = turner::password_algorithms;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 16;

	using algorithm = turner::password_algorithm;

	static constexpr sample<value_type> samples[] =
	{
		{"00010000", {{algorithm::md5}}},
		{"00020000" "00010000", {{algorithm::sha256, algorithm::md5}}},
		{"00030000" "00020000", {{static_cast<algorithm>(3), algorithm::sha256}}},
	};
};

struct channel_number
{
	using value_type = turner::channel_number_value;
	using native_type = uint16_t;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"40000000", 0x4000},
		{"4fff0000", 0x4fff},
	};
};

struct address_error_code
{
	using value_type = turner::address_error_code_value;
	using native_type = turner::address_error;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4 + 763;

	using errc = turner::protocol_errc;
	using family = turner::address_family;

	static constexpr sample<value_type> samples[] =
	{
		{"01000428", {.family = family::v4, .error = {errc::address_family_not_supported, ""}}},
		{"02000508" "4f7574", {.family = family::v6, .error = {errc::insufficient_capacity, "Out"}}},
	};
};

struct msturn_error_code
{
	using value_type = turner::msturn::error_code_value;
	using native_type = turner::protocol_error;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 0xfffc;

	static constexpr sample<value_type> samples[] =
	{
		{"00000300", {turner::protocol_errc::try_alternate, ""}},
		{"00000401" "556e6175" "74686f72" "697a6564", {turner::protocol_errc::unauthorized, "Unauthorized"}},
	};
};

struct msturn_unknown_attributes
{
	using value_type = turner::msturn::unknown_attributes_value;
	using native_type = turner::attribute_type_list;
	static constexpr size_t min_size = 2;
	static constexpr size_t max_size = 0xfffe;

	static constexpr sample<value_type> samples[] =
	{
		{"000a" "8000", {{0x000a, 0x8000}}},
		{"000a" "8000" "0001" "c001", {{0x000a, 0x8000, 0x0001, 0xc001}}},
	};
};

struct msturn_protocol_version
{
	using value_type = turner::msturn::protocol_version_value;
	using native_type = turner::msturn::protocol_version;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	static constexpr sample<value_type> samples[] =
	{
		{"00000001", turner::msturn::protocol_version::v1},
		{"00000006", turner::msturn::protocol_version::v6},
		{"00000007", static_cast<turner::msturn::protocol_version>(7)},
	};
};

static_assert(turner::msturn::protocol_version::v6 >= turner::msturn::protocol_version::with_ipv6);
static_assert(turner::msturn::protocol_version::v3 < turner::msturn::protocol_version::with_ipv6);

struct msturn_service_quality
{
	using value_type = turner::msturn::service_quality_value;
	using native_type = turner::msturn::service_quality;
	static constexpr size_t min_size = 4;
	static constexpr size_t max_size = 4;

	using stream = turner::msturn::stream_type;
	using delivery = turner::msturn::delivery;

	static constexpr sample<value_type> samples[] =
	{
		{"00010000", {.stream = stream::audio, .quality = delivery::best_effort}},
		{"00040001", {.stream = stream::data, .quality = delivery::reliable}},
		{"00090007", {.stream = static_cast<stream>(9), .quality = static_cast<delivery>(7)}},
	};
};

struct msturn_sequence_number
{
	using value_type = turner::msturn::sequence_number_value;
	using native_type = turner::msturn::sequence_number;
	static constexpr size_t min_size = 24;
	static constexpr size_t max_size = 24;

	static constexpr auto connection_id = turner_test::bytes("0102030405060708090a0b0c0d0e0f1011121314");

	static constexpr sample<value_type> samples[] =
	{
		{"01020304" "05060708" "090a0b0c" "0d0e0f10" "11121314" "00000005", {connection_id, 5}},
	};
};

constexpr std::span<const std::byte> no_message;
constexpr std::byte guard{0xa5};

template <typename T>
bool same (const T &a, const T &b)
{
	return a == b;
}

template <size_t Extent>
bool same (std::span<const std::byte, Extent> a, std::span<const std::byte, Extent> b)
{
	return std::ranges::equal(a, b);
}

bool same (const turner::protocol_error &a, const turner::protocol_error &b)
{
	return a.code == b.code && a.reason == b.reason;
}

bool same (const turner::address_error &a, const turner::address_error &b)
{
	return a.family == b.family && same(a.error, b.error);
}


template <typename Value>
std::vector<std::byte> written (
	const typename Value::native_type &native,
	std::span<const std::byte> message = no_message)
{
	std::vector<std::byte> out(Value::size(native) + 1, guard);
	Value::write(message, std::span{out}.first(out.size() - 1), native);
	return out;
}

template <typename T>
std::vector<std::byte> message_of ()
{
	if constexpr (requires { T::message; })
	{
		return turner_test::to_bytes(T::message);
	}
	else
	{
		return {};
	}
}

std::vector<std::byte> with_guard (std::string_view hex)
{
	auto bytes = turner_test::to_bytes(hex);
	bytes.push_back(guard);
	return bytes;
}

TEMPLATE_TEST_CASE("value", "",
	uint16,
	uint32,
	seconds,
	string,
	limited_string,
	bytes,
	fixed_bytes,
	flag,
	address_family,
	transport_protocol,
	endpoint,
	xor_endpoint,
	error_code,
	unlimited_error_code,
	unknown_attributes,
	ipv6_endpoint,
	password_algorithm,
	password_algorithms,
	channel_number,
	address_error_code,
	msturn_error_code,
	msturn_unknown_attributes,
	msturn_protocol_version,
	msturn_service_quality,
	msturn_sequence_number)
{
	using value_type = typename TestType::value_type;

	static_assert(turner::attribute_value<value_type>);
	static_assert(std::same_as<typename value_type::native_type, typename TestType::native_type>);
	static_assert(value_type::min_size == TestType::min_size);
	static_assert(value_type::max_size == TestType::max_size);

	const auto s = GENERATE(from_range(TestType::samples));
	CAPTURE(s.wire);
	const auto wire = turner_test::to_bytes(s.wire);
	const auto message = message_of<TestType>();

	SECTION("sample fits size limits")
	{
		CHECK(wire.size() >= value_type::min_size);
		CHECK(wire.size() <= value_type::max_size);
	}

	SECTION("size")
	{
		CHECK(value_type::size(s.native) == wire.size());
	}

	SECTION("read")
	{
		const auto native = value_type::read(message, wire);
		REQUIRE(native);
		CHECK(same(*native, s.native));
	}

	SECTION("write")
	{
		CHECK(written<value_type>(s.native, message) == with_guard(s.wire));
	}
}

TEST_CASE("value")
{
	SECTION("seconds: write saturates")
	{
		CHECK(written<turner::seconds_value>(-1s) == with_guard("00000000"));
		CHECK(written<turner::seconds_value>(std::chrono::seconds::min()) == with_guard("00000000"));
		CHECK(written<turner::seconds_value>(std::chrono::seconds{0x1'0000'0000}) == with_guard("ffffffff"));
		CHECK(written<turner::seconds_value>(std::chrono::seconds::max()) == with_guard("ffffffff"));
		CHECK(written<turner::seconds_value>(std::chrono::hours{1}) == with_guard("00000e10"));
	}

	SECTION("string: read refers to input")
	{
		const auto wire = turner_test::to_bytes("616c696365");
		const auto value = turner::string_value<>::read(no_message, wire);
		REQUIRE(value);
		CHECK(static_cast<const void *>(value->data()) == wire.data());
		CHECK(value->size() == wire.size());
	}

	SECTION("bytes: read refers to input")
	{
		const auto wire = turner_test::to_bytes("0102030405060708");

		const auto dynamic = turner::bytes_value<>::read(no_message, std::span{wire}.first(3));
		REQUIRE(dynamic);
		CHECK(dynamic->data() == wire.data());
		CHECK(dynamic->size() == 3);

		const auto fixed = turner::bytes_value<8>::read(no_message, wire);
		REQUIRE(fixed);
		CHECK(fixed->data() == wire.data());
		CHECK(fixed->size() == 8);
	}

	SECTION("enum: read ignores reserved bytes")
	{
		const auto wire = turner_test::to_bytes("02ffffff");
		CHECK(turner::address_family_value::read(no_message, wire) == turner::address_family::v6);
	}

	SECTION("enum: read rejects invalid address family")
	{
		const auto hex = GENERATE("00000000"sv, "03000000"sv, "ff000000"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(turner::address_family_value::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("enum: read rejects invalid transport protocol")
	{
		const auto hex = GENERATE("00000000"sv, "05000000"sv, "07000000"sv, "10000000"sv, "12000000"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(turner::transport_protocol_value::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("enum: write clears reserved bytes")
	{
		std::vector<std::byte> out(5, guard);
		turner::address_family_value::write(no_message, std::span{out}.first(4), turner::address_family::v6);
		CHECK(out == with_guard("02000000"));
	}

	SECTION("transport_address: from endpoint")
	{
		const pal::net::ip::udp::endpoint v4{ipv4, stun_port};
		const turner::transport_address from_v4 = v4;
		CHECK(from_v4 == turner::transport_address{ipv4, stun_port});

		const pal::net::ip::udp::endpoint v6{ipv6, stun_port};
		const turner::transport_address from_v6 = v6;
		CHECK(from_v6 == turner::transport_address{ipv6, stun_port});
	}

	SECTION("endpoint: read rejects length other than IPv4 or IPv6")
	{
		const auto hex = GENERATE(
			"00010d96" "c0000201" "00"sv,
			"00010d96" "c0000201" "00000000"sv,
			"00020d96" "20010db8" "00000000" "00000000" "000000"sv
		);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		const auto message = message_of<xor_endpoint>();
		const auto expected = turner::make_unexpected(turner::errc::unexpected_attribute_length);
		CHECK(turner::endpoint_value::read(no_message, wire) == expected);
		CHECK(turner::xor_endpoint_value::read(message, wire) == expected);
	}

	SECTION("endpoint: read rejects family not matching length")
	{
		const auto hex = GENERATE(
			"00000d96" "c0000201"sv,
			"00020d96" "c0000201"sv,
			"00030d96" "c0000201"sv,
			"00010d96" "20010db8" "00000000" "00000000" "00000001"sv,
			"00030d96" "20010db8" "00000000" "00000000" "00000001"sv
		);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		const auto message = message_of<xor_endpoint>();
		const auto expected = turner::make_unexpected(turner::errc::unexpected_attribute_value);
		CHECK(turner::endpoint_value::read(no_message, wire) == expected);
		CHECK(turner::xor_endpoint_value::read(message, wire) == expected);
	}

	SECTION("endpoint: read ignores reserved byte")
	{
		const auto wire = turner_test::to_bytes("ff010d96" "c0000201");
		CHECK(turner::endpoint_value::read(no_message, wire) == turner::transport_address{ipv4, stun_port});

		const auto xor_wire = turner_test::to_bytes("ff01a147" "e112a643");
		CHECK(turner::xor_endpoint_value::read(message_of<xor_endpoint>(), xor_wire)
			==
			turner::transport_address{ipv4, xor_endpoint::rfc5769_port}
		);
	}

	SECTION("endpoint: write ignores IPv6 scope ID")
	{
		const auto scoped = pal::net::ip::make_address_v6(ipv6.to_bytes(), 7);
		CHECK(written<turner::endpoint_value>({scoped, stun_port})
			==
			with_guard("00020d96" "20010db8" "00000000" "00000000" "00000001")
		);
	}

	SECTION("xor endpoint: MS-TURN IPv4 transaction ID")
	{
		const auto message = turner_test::to_bytes("00010000" "aabbccdd" "00000000" "00000000" "00000000");
		const auto hex = "0001bb99" "bb99ff99"sv;
		const turner::transport_address native
		{
			pal::net::ip::make_address_v4(0x1122'3344),
			pal::net::ip::port_type{0x1122},
		};
		CHECK(turner::xor_endpoint_value::read(message, turner_test::to_bytes(hex)) == native);
		CHECK(written<turner::xor_endpoint_value>(native, message) == with_guard(hex));
	}

	SECTION("xor endpoint: MS-TURN IPv6 transaction ID")
	{
		const auto message = turner_test::to_bytes("00010000" "11223344" "55667788" "99aabbcc" "ddeeff00");
		const auto hex = "00025577" "31233efc" "444444cc" "cccccc44" "444444cc"sv;
		const turner::transport_address native
		{
			pal::net::ip::make_address_v6(pal::net::ip::address_v6::bytes_type{
				0x20, 0x01, 0x0d, 0xb8, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
			}),
			pal::net::ip::port_type{0x4455},
		};
		CHECK(turner::xor_endpoint_value::read(message, turner_test::to_bytes(hex)) == native);
		CHECK(written<turner::xor_endpoint_value>(native, message) == with_guard(hex));
	}

	SECTION("error code: read rejects class or number out of range")
	{
		const auto hex = GENERATE("00000000"sv, "00000200"sv, "00000700"sv, "00000464"sv, "000004ff"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(turner::error_code_value<>::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("error code: read ignores reserved bits")
	{
		const auto wire = turner_test::to_bytes("fffffc01");
		const auto value = turner::error_code_value<>::read(no_message, wire);
		REQUIRE(value);
		CHECK(same(*value, {turner::protocol_errc::unauthorized, ""}));
	}

	SECTION("error code: write bare code with its reason phrase")
	{
		CHECK(written<turner::error_code_value<>>(turner::protocol_errc::stale_nonce)
			==
			with_guard("00000426" "5374616c" "65204e6f" "6e6365")
		);
	}

	SECTION("error code: read refers to input")
	{
		const auto wire = turner_test::to_bytes("00000401" "556e6175" "74686f72" "697a6564");
		const auto value = turner::error_code_value<>::read(no_message, wire);
		REQUIRE(value);
		CHECK(static_cast<const void *>(value->reason.data()) == wire.data() + 4);
	}

	SECTION("unknown attributes: read rejects odd length")
	{
		const auto wire = turner_test::to_bytes("000a00");
		CHECK(turner::unknown_attributes_value::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_length)
		);
	}

	SECTION("unknown attributes: read drops types beyond capacity")
	{
		const auto wire = turner_test::to_bytes("0001" "0002" "0003" "0004" "0005");
		CHECK(turner::unknown_attributes_value::read(no_message, wire)
			==
			turner::attribute_type_list{{1, 2, 3, 4}}
		);
	}

	SECTION("unknown attributes: construction limited to capacity")
	{
		static_assert(std::constructible_from<turner::attribute_type_list, const uint16_t (&)[4]>);
		static_assert(!std::constructible_from<turner::attribute_type_list, const uint16_t (&)[5]>);
	}

	SECTION("unknown attributes: push_back beyond capacity is dropped")
	{
		turner::attribute_type_list types{{1, 2, 3, 4}};
		types.push_back(5);
		CHECK(types.size() == turner::attribute_type_list::capacity);
		CHECK(types == turner::attribute_type_list{{1, 2, 3, 4}});
	}

	SECTION("unknown attributes: true when not empty")
	{
		CHECK_FALSE(turner::attribute_type_list{});
		CHECK(turner::attribute_type_list{{0x000a}});
	}

	SECTION("password algorithm: read rejects unknown algorithm or parameters")
	{
		const auto hex = GENERATE("00000000"sv, "00030000"sv, "00010004"sv, "00020001"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(turner::password_algorithm_value::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("password algorithms: read rejects parameters or partial entry")
	{
		const auto parameters = turner_test::to_bytes("00020000" "00010004");
		CHECK(turner::password_algorithms_value::read(no_message, parameters)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
		const auto partial = turner_test::to_bytes("00020000" "0001");
		CHECK(turner::password_algorithms_value::read(no_message, partial)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_length)
		);
	}

	SECTION("password algorithms: construction limited to capacity")
	{
		using algorithm = turner::password_algorithm;
		static_assert(std::constructible_from<turner::password_algorithms, const algorithm (&)[4]>);
		static_assert(!std::constructible_from<turner::password_algorithms, const algorithm (&)[5]>);
		static_assert(!std::is_default_constructible_v<turner::password_algorithms>);
	}

	SECTION("channel number: read rejects number out of range")
	{
		const auto hex = GENERATE("00000000"sv, "3fff0000"sv, "50000000"sv, "ffff0000"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(turner::channel_number_value::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("channel number: read ignores reserved bytes")
	{
		const auto wire = turner_test::to_bytes("4000ffff");
		CHECK(turner::channel_number_value::read(no_message, wire) == 0x4000);
	}

	SECTION("address error code: read rejects invalid family or code")
	{
		const auto hex = GENERATE("00000428"sv, "03000428"sv, "01000228"sv, "01000464"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(turner::address_error_code_value::read(no_message, wire)
			==
			turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("address error code: read ignores reserved bits")
	{
		const auto wire = turner_test::to_bytes("02fffd08");
		const auto value = turner::address_error_code_value::read(no_message, wire);
		REQUIRE(value);
		CHECK(same(*value, {turner::address_family::v6, {turner::protocol_errc::insufficient_capacity, ""}}));
	}

	SECTION("msturn error code: write pads reason with spaces")
	{
		CHECK(written<turner::msturn::error_code_value>(turner::protocol_errc::stale_nonce)
			==
			with_guard("00000426" "5374616c" "65204e6f" "6e636520")
		);
		CHECK(written<turner::msturn::error_code_value>(turner::protocol_errc::forbidden)
			==
			with_guard("00000403" "466f7262" "69646465" "6e202020")
		);
	}

	SECTION("msturn error code: read accepts unpadded reason")
	{
		const auto wire = turner_test::to_bytes("00000403" "466f7262" "69646465" "6e");
		const auto value = turner::msturn::error_code_value::read(no_message, wire);
		REQUIRE(value);
		CHECK(same(*value, {turner::protocol_errc::forbidden, "Forbidden"}));
	}

	SECTION("msturn unknown attributes: write repeats last type when odd")
	{
		CHECK(written<turner::msturn::unknown_attributes_value>({{0x000a}}) == with_guard("000a" "000a"));
		CHECK(written<turner::msturn::unknown_attributes_value>({{0x000a, 0x8000, 0x0001}})
			==
			with_guard("000a" "8000" "0001" "0001")
		);
	}

	SECTION("msturn unknown attributes: read accepts unpadded list")
	{
		const auto wire = turner_test::to_bytes("000a");
		CHECK(turner::msturn::unknown_attributes_value::read(no_message, wire)
			==
			turner::attribute_type_list{{0x000a}}
		);
	}
}

} // namespace
