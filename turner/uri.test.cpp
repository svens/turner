#include <turner/uri.hpp>
#include <pal/net/ip/address_v4.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <span>
#include <string>
#include <utility>

namespace
{

using pal::net::ip::port_type;
using turner::default_port;
using turner::default_transport;
using turner::transport;
using turner::uri_scheme;
using turner::uri_view;

static_assert(default_port(uri_scheme::stun) == port_type::stun);
static_assert(default_port(uri_scheme::stuns) == port_type::stuns);
static_assert(default_port(uri_scheme::turn) == port_type::turn);
static_assert(default_port(uri_scheme::turns) == port_type::turns);
static_assert(default_transport(uri_scheme::stun) == transport::datagram);
static_assert(default_transport(uri_scheme::stuns) == transport::stream);
static_assert(default_transport(uri_scheme::turn) == transport::datagram);
static_assert(default_transport(uri_scheme::turns) == transport::stream);

static_assert(noexcept(uri_view::make({})));
static_assert(noexcept(std::declval<uri_view &>().from_chars(nullptr, nullptr)));
static_assert(noexcept(std::declval<const uri_view &>().to_chars(nullptr, nullptr)));
static_assert(noexcept(std::declval<const uri_view &>().max_size()));
static_assert(noexcept(std::declval<const uri_view &>() == std::declval<const uri_view &>()));

constexpr auto address_1234 = pal::net::ip::make_address_v4(pal::net::ip::address_v4::bytes_type{1, 2, 3, 4});
constexpr auto loopback = pal::net::ip::address_v6::loopback;
constexpr auto scoped_loopback = pal::net::ip::make_address_v6(loopback.to_bytes(), 1);
constexpr auto address_1_0_2 = pal::net::ip::make_address_v6(
	pal::net::ip::address_v6::bytes_type{0, 1, 0, 0, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7}
);
constexpr auto address_ffff_0_0 = pal::net::ip::make_address_v6(
	pal::net::ip::address_v6::bytes_type{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0, 0, 0, 0}
);
constexpr auto address_mapped_1234 = pal::net::ip::make_address_v6(
	pal::net::ip::address_v6::bytes_type{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 1, 2, 3, 4}
);
constexpr auto address_all_ffff = pal::net::ip::make_address_v6(
	pal::net::ip::address_v6::bytes_type{
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	}
);

constexpr pal::uri_host reg_name (std::string_view text) noexcept
{
	return {.value = text};
}

constexpr pal::uri_host ip (pal::net::ip::address address) noexcept
{
	return {.value = address};
}

constexpr uri_view stun (
	pal::uri_host host,
	port_type port = port_type::stun,
	transport t = transport::datagram) noexcept
{
	return {.scheme = uri_scheme::stun, .host = host, .port = port, .transport = t};
}

constexpr uri_view stuns (
	pal::uri_host host,
	port_type port = port_type::stuns,
	transport t = transport::stream) noexcept
{
	return {.scheme = uri_scheme::stuns, .host = host, .port = port, .transport = t};
}

constexpr uri_view turn (
	pal::uri_host host,
	port_type port = port_type::turn,
	transport t = transport::datagram) noexcept
{
	return {.scheme = uri_scheme::turn, .host = host, .port = port, .transport = t};
}

constexpr uri_view turns (
	pal::uri_host host,
	port_type port = port_type::turns,
	transport t = transport::stream) noexcept
{
	return {.scheme = uri_scheme::turns, .host = host, .port = port, .transport = t};
}

constexpr auto h = reg_name("h");

struct parse_case
{
	std::string_view text;
	uri_view expected;
	std::string_view canonical{};
};

constexpr parse_case parse_success[] =
{
	{"stun:h", stun(h)},
	{"stuns:h", stuns(h)},
	{"turn:h", turn(h)},
	{"turns:h", turns(h)},
	{"STUN:h", stun(h), "stun:h"},
	{"TuRnS:h", turns(h), "turns:h"},
	{"stun:H", stun(reg_name("H"))},
	{"stun:1.2.3.4", stun(ip(address_1234))},
	{"turn:[::1]", turn(ip(loopback))},
	{"stun:%41b", stun(reg_name("%41b"))},
	{"stun:a=b&c;d", stun(reg_name("a=b&c;d"))},
	{"stun:01.2.3.4", stun(reg_name("01.2.3.4"))},
	{"stun:1.2.3", stun(reg_name("1.2.3"))},
	{"stun:example.com", stun(reg_name("example.com"))},
	{"stun:h:0", stun(h, port_type::unspecified)},
	{"stun:h:65535", stun(h, port_type{65535})},
	{"stun:h:5349", stun(h, port_type::stuns)},
	{"stuns:h:3478", stuns(h, port_type::stun)},
	{"stun:h:0000000000000000000003478", stun(h), "stun:h"},
	{"stun:h:3478", stun(h), "stun:h"},
	{"stuns:h:5349", stuns(h), "stuns:h"},
	{"turn:h:3478", turn(h), "turn:h"},
	{"turns:h:5349", turns(h), "turns:h"},
	{"stun:h:", stun(h), "stun:h"},
	{"turn:[::1]:", turn(ip(loopback)), "turn:[::1]"},
	{"turn:h:?transport=tcp", turn(h, port_type::turn, transport::stream), "turn:h?transport=tcp"},
	{"stun:h?transport=udp", stun(h), "stun:h"},
	{"stun:h?transport=tcp", stun(h, port_type::stun, transport::stream)},
	{"stuns:h?transport=udp", stuns(h, port_type::stuns, transport::datagram)},
	{"stuns:h?transport=tcp", stuns(h), "stuns:h"},
	{"turn:h?transport=udp", turn(h), "turn:h"},
	{"turn:h?transport=tcp", turn(h, port_type::turn, transport::stream)},
	{"turns:h?transport=udp", turns(h, port_type::turns, transport::datagram)},
	{"turns:h?transport=tcp", turns(h), "turns:h"},
	{"turn:h?TRANSPORT=TCP", turn(h, port_type::turn, transport::stream), "turn:h?transport=tcp"},
	{"turn:h?Transport=Tcp", turn(h, port_type::turn, transport::stream), "turn:h?transport=tcp"},
	{"turn:h:443?transport=tcp", turn(h, port_type::https, transport::stream)},
	{"turns:[::1]:443?transport=udp", turns(ip(loopback), port_type::https, transport::datagram)},
	{"turn:1.2.3.4:443?transport=tcp", turn(ip(address_1234), port_type::https, transport::stream)},
	{"turn:[::1]?transport=tcp", turn(ip(loopback), port_type::turn, transport::stream)},
	{
		"turns:[ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff]:65535?transport=udp",
		turns(ip(address_all_ffff), port_type{65535}, transport::datagram),
	},
};

constexpr parse_case parse_roundtrip[] =
{
	{"turn:[1::2:3:4:5:6:7]", turn(ip(address_1_0_2))},
	{"turn:[::ffff:0:0]", turn(ip(address_ffff_0_0))},
	{"stun:[::ffff:1.2.3.4]", stun(ip(address_mapped_1234))},
};

constexpr std::string_view parse_failure[] =
{
	"",
	std::string_view{},
	"stun",
	"h",
	":h",
	"stunx:h",
	"sturn:h",
	"stun :h",
	" stun:h",
	"http:h",
	"stun:",
	"stun://h",
	"stun:u@h",
	"stun:h/p",
	"stun:h#f",
	"stun:h x",
	"stun::3478",
	"stun:?transport=udp",
	"stun:h:65536",
	"stun:h:99999999999",
	"stun:h:x",
	"stun:h:+1",
	"stun:h:-1",
	"stun:h:34a",
	"stun:h:3478:1",
	"stun:h:3478?",
	"stun:h:?",
	"stun:h: 1",
	"stun:h ",
	"stun:h:3478?transport=",
	"stun:[fe80::1%25eth0]",
	"stun:[v1.x]",
	"stun:[::1",
	"stun:[::1]x",
	"stun:h%",
	"stun:h%4",
	"stun:h%zz",
	"stun:h?",
	"stun:h?x=udp",
	"stun:h?Transport",
	"stun:h?transport",
	"turn:h?transport=",
	"turn:h?transport=sctp",
	"turn:h?transport=udpx",
	"turn:h?transport=ud",
	"turn:h?transport=udp&x=1",
	"turn:h?transport=udp:1",
	"turn:h?transport=udp?transport=udp",
	std::string_view{"stun:h\0", 7},
	std::string_view{"stun\0:h", 7},
};

constexpr uri_view format_failure[] =
{
	stun(reg_name("")),
	stun(reg_name("1.2.3.4")),
	stun(reg_name("[::1]")),
	stun(reg_name("a b")),
	stun(reg_name("h:1")),
	stun(reg_name("%zz")),
	stun(ip(scoped_loopback)),
};

struct compare_case
{
	std::string_view a;
	std::string_view b;
	bool equal;
};

constexpr compare_case compare_texts[] =
{
	{"stun:h:3478", "stun:h", true},
	{"stuns:h:5349", "stuns:h", true},
	{"turn:h?transport=udp", "turn:h", true},
	{"turns:h?transport=tcp", "turns:h", true},
	{"STUN:h", "stun:h", true},
	{"turn:[::1]", "turn:[0::1]", true},
	{"stun:H", "stun:h", false},
	{"stun:%41", "stun:A", false},
	{"stun:h", "turn:h", false},
	{"stun:h", "stun:h:1", false},
	{"stun:h", "stun:h?transport=tcp", false},
	{"stun:1.2.3.4", "stun:1.2.3.5", false},
};

pal::result<uri_view> check_make (std::string_view text)
{
	uri_view value;
	const auto [ptr, ec] = value.from_chars(text.data(), text.data() + text.size());
	auto made = uri_view::make(text);
	if (ec == std::errc{})
	{
		CHECK(ptr == text.data() + text.size());
		REQUIRE(made);
		CHECK(*made == value);
	}
	else
	{
		CHECK(ec == std::errc::invalid_argument);
		CHECK(made.error() == std::errc::invalid_argument);
	}
	return made;
}

std::string check_to_chars (const uri_view &value)
{
	std::string buf(value.max_size(), '\0');
	const auto [ptr, ec] = value.to_chars(buf.data(), buf.data() + buf.size());
	REQUIRE(ec == std::errc{});
	buf.resize(static_cast<size_t>(ptr - buf.data()));

	std::string exact(buf.size(), '\0');
	const auto [exact_ptr, exact_ec] = value.to_chars(exact.data(), exact.data() + exact.size());
	REQUIRE(exact_ec == std::errc{});
	CHECK(exact_ptr == exact.data() + exact.size());
	CHECK(exact == buf);

	for (size_t size = 0; size < buf.size(); ++size)
	{
		CAPTURE(size);
		std::string small(size, '\0');
		const auto [small_ptr, small_ec] = value.to_chars(small.data(), small.data() + small.size());
		CHECK(small_ec == std::errc::value_too_large);
		CHECK(small_ptr == small.data() + small.size());
	}

	return buf;
}

std::string check_roundtrip (std::string_view text, const uri_view &expected)
{
	const auto uri = check_make(text);
	REQUIRE(uri);
	CHECK(*uri == expected);

	auto formatted = check_to_chars(expected);
	const auto reparsed = uri_view::make(formatted);
	REQUIRE(reparsed);
	CHECK(*reparsed == expected);
	return formatted;
}

TEST_CASE("uri")
{
	SECTION("parse success")
	{
		const auto &row = GENERATE(from_range(std::span{parse_success}));
		CAPTURE(row.text);
		const auto canonical = row.canonical.empty() ? row.text : row.canonical;
		CHECK(check_roundtrip(row.text, row.expected) == canonical);
	}

	SECTION("parse roundtrip")
	{
		const auto &row = GENERATE(from_range(std::span{parse_roundtrip}));
		CAPTURE(row.text);
		check_roundtrip(row.text, row.expected);
	}

	SECTION("parse failure")
	{
		const auto &text = GENERATE(from_range(std::span{parse_failure}));
		CAPTURE(text);
		CHECK_FALSE(check_make(text));
	}

	SECTION("format failure")
	{
		const auto &value = GENERATE(from_range(std::span{format_failure}));
		for (size_t size = 0; size <= value.max_size(); ++size)
		{
			CAPTURE(size);
			std::string buf(size, '\0');
			const auto [ptr, ec] = value.to_chars(buf.data(), buf.data() + buf.size());
			CHECK(ec == std::errc::invalid_argument);
			CHECK(ptr == buf.data() + buf.size());
		}
	}

	SECTION("compare")
	{
		const auto &[a, b, equal] = GENERATE(from_range(std::span{compare_texts}));
		CAPTURE(a, b);
		const auto left = uri_view::make(a);
		const auto right = uri_view::make(b);
		REQUIRE(left);
		REQUIRE(right);
		CHECK((*left == *right) == equal);
	}

	SECTION("host refers to parsed text")
	{
		constexpr std::string_view text = "stun:example.com:1";
		const auto uri = uri_view::make(text);
		REQUIRE(uri);
		const auto *host = std::get_if<std::string_view>(&uri->host.value);
		REQUIRE(host != nullptr);
		CHECK(host->data() == text.data() + 5);
	}
}

} // namespace
