#include <turner/codec/test.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
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
	static pal::result<int> read (std::span<const std::byte>, std::span<const std::byte>) noexcept;
	static size_t size (const int &) noexcept;
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

struct with_throwing_read: valid_value
{
	static pal::result<int> read (std::span<const std::byte>, std::span<const std::byte>);
};

struct with_other_read_result: valid_value
{
	static int read (std::span<const std::byte>, std::span<const std::byte>) noexcept;
};

struct with_throwing_size: valid_value
{
	static size_t size (const int &);
};

struct with_other_size_result: valid_value
{
	static int size (const int &) noexcept;
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
static_assert(!turner::attribute_value<with_throwing_read>);
static_assert(!turner::attribute_value<with_other_read_result>);
static_assert(!turner::attribute_value<with_throwing_size>);
static_assert(!turner::attribute_value<with_other_size_result>);
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

template <typename Value>
std::vector<std::byte> written (const typename Value::native_type &native)
{
	std::vector<std::byte> out(Value::size(native) + 1, guard);
	Value::write(no_message, std::span{out}.first(out.size() - 1), native);
	return out;
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
	transport_protocol)
{
	using value_type = typename TestType::value_type;

	static_assert(turner::attribute_value<value_type>);
	static_assert(std::same_as<typename value_type::native_type, typename TestType::native_type>);
	static_assert(value_type::min_size == TestType::min_size);
	static_assert(value_type::max_size == TestType::max_size);

	const auto s = GENERATE(from_range(TestType::samples));
	CAPTURE(s.wire);
	const auto wire = turner_test::to_bytes(s.wire);

	SECTION("sample fits size limits")
	{
		CHECK(wire.size() >= value_type::min_size);
		CHECK(wire.size() <= value_type::max_size);
	}

	SECTION("read")
	{
		const auto native = value_type::read(no_message, wire);
		REQUIRE(native);
		CHECK(same(*native, s.native));
	}

	SECTION("size")
	{
		CHECK(value_type::size(s.native) == wire.size());
	}

	SECTION("write")
	{
		CHECK(written<value_type>(s.native) == with_guard(s.wire));
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
		CHECK(
			turner::address_family_value::read(no_message, wire)
			== turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("enum: read rejects invalid transport protocol")
	{
		const auto hex = GENERATE("00000000"sv, "05000000"sv, "07000000"sv, "10000000"sv, "12000000"sv);
		CAPTURE(hex);
		const auto wire = turner_test::to_bytes(hex);
		CHECK(
			turner::transport_protocol_value::read(no_message, wire)
			== turner::make_unexpected(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("enum: write clears reserved bytes")
	{
		std::vector<std::byte> out(5, guard);
		turner::address_family_value::write(no_message, std::span{out}.first(4), turner::address_family::v6);
		CHECK(out == with_guard("02000000"));
	}
}

} // namespace
