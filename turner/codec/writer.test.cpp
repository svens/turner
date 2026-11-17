#include <turner/codec/test.hpp>
#include <turner/codec/writer.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <turner/codec/value.hpp>
#include <pal/net/ip/address_v4.hpp>
#include <pal/net/ip/port_type.hpp>
#include <pal/result.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{

using namespace std::chrono_literals;

template <auto Entity>
using writer = turner::message_writer<Entity>;

template <auto Entity>
concept nameable = requires
{
	typename writer<Entity>;
};

static_assert(std::same_as<writer<turner::turn::allocate>, writer<turner::turn::allocate.request>>);
static_assert(std::same_as<writer<turner::turn::binding.request>, writer<turner::stun::binding.request>>);
static_assert(std::same_as<writer<turner::turn::allocate>::protocol_type, turner::turn>);
static_assert(nameable<turner::stun::binding.indication>);
static_assert(!nameable<turner::stun::binding>);

template <typename Writer>
concept constructible = std::constructible_from<
	Writer,
	std::span<std::byte>,
	const typename Writer::protocol_type::transaction_id &
>;

static_assert(constructible<writer<turner::stun::binding.request>>);
static_assert(constructible<writer<turner::stun::binding.indication>>);
static_assert(constructible<writer<turner::msturn::send_request>>);
static_assert(!constructible<writer<turner::turn::allocate.success>>);
static_assert(!constructible<writer<turner::turn::allocate.error>>);
static_assert(!std::is_default_constructible_v<writer<turner::turn::allocate>>);
static_assert(
	std::is_nothrow_constructible_v<
		writer<turner::turn::allocate>,
		std::span<std::byte>,
		const turner::turn::transaction_id &
	>
);

template <typename Writer, auto Attribute, typename Value>
concept can_write = requires(Writer writer, const Value &value)
{
	writer.write(Attribute, value);
};

template <typename Writer, auto Attribute>
concept can_write_flag = requires(Writer writer)
{
	writer.write(Attribute);
};

template <typename Writer>
concept can_finish_lvalue = requires(Writer &writer)
{
	writer.finish();
};

template <typename Writer>
concept can_finish_rvalue = requires(Writer &&writer)
{
	std::move(writer).finish();
};

using stun_writer = writer<turner::stun::binding.request>;
using turn_writer = writer<turner::turn::allocate>;
using msturn_writer = writer<turner::msturn::allocate>;

static_assert(can_write<stun_writer &, turner::stun::software, std::string_view>);
static_assert(can_write<stun_writer &&, turner::stun::software, std::string_view>);
static_assert(can_write<turn_writer &, turner::stun::software, std::string_view>);
static_assert(can_write<turn_writer &, turner::turn::lifetime, std::chrono::seconds>);
static_assert(can_write<msturn_writer &, turner::msturn::username, std::string_view>);
static_assert(!can_write<const stun_writer &, turner::stun::software, std::string_view>);
static_assert(!can_write<stun_writer &, turner::turn::lifetime, std::chrono::seconds>);
static_assert(!can_write<msturn_writer &, turner::stun::software, std::string_view>);
static_assert(!can_write<msturn_writer &, turner::turn::lifetime, std::chrono::seconds>);
static_assert(!can_write<turn_writer &, turner::turn::dont_fragment, std::true_type>);
static_assert(can_write_flag<turn_writer &, turner::turn::dont_fragment>);
static_assert(!can_write_flag<const turn_writer &, turner::turn::dont_fragment>);
static_assert(!can_write_flag<turn_writer &, turner::turn::lifetime>);
static_assert(can_finish_rvalue<turn_writer>);
static_assert(!can_finish_lvalue<turn_writer>);
static_assert(std::same_as<decltype(std::declval<turn_writer &>().write(turner::turn::lifetime, 0s)), turn_writer &>);
static_assert(std::same_as<decltype(std::declval<turn_writer>().write(turner::turn::lifetime, 0s)), turn_writer &&>);
static_assert(std::same_as<decltype(std::declval<turn_writer &>().write(turner::turn::dont_fragment)), turn_writer &>);
static_assert(noexcept(std::declval<turn_writer &>().write(turner::turn::lifetime, std::chrono::seconds{})));
static_assert(noexcept(std::declval<turn_writer &>().write(turner::turn::dont_fragment)));
static_assert(noexcept(std::declval<turn_writer>().finish()));
static_assert(
	std::same_as<
		decltype(std::declval<turn_writer>().finish()),
		pal::result<std::span<const std::byte>>
	>
);

constexpr std::byte filler{0xff};

constexpr turner::stun::transaction_id stun_id = turner_test::bytes("0102030405060708090a0b0c");
constexpr turner::msturn::transaction_id msturn_id = turner_test::bytes("2112a442" "0102030405060708090a0b0c");

constexpr auto test_ipv4 = pal::net::ip::make_address_v4(0xc000'0201);
constexpr turner::transport_address peer_address{test_ipv4, pal::net::ip::port_type{32853}};
constexpr turner::transport_address server_address{test_ipv4, pal::net::ip::port_type{3478}};

std::vector<std::byte> make_buffer (size_t size = 256)
{
	return std::vector<std::byte>(size, filler);
}

template <size_t Size>
std::array<std::byte, Size> make_array () noexcept
{
	std::array<std::byte, Size> buffer{};
	buffer.fill(filler);
	return buffer;
}

std::span<const std::byte> as_bytes (std::string_view text) noexcept
{
	return std::as_bytes(std::span{text});
}

pal::unexpected failed (turner::errc error) noexcept
{
	return turner::make_unexpected(error);
}

std::string hex (std::span<const std::byte> bytes)
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

template <typename Protocol, auto Entity>
turner::message_reader<Entity> parse_as (std::span<const std::byte> message, std::vector<std::byte> &buffer)
{
	buffer.assign(message.begin(), message.end());
	const auto parsed = Protocol::parse(buffer, buffer.size());
	REQUIRE(parsed);
	const auto reader = parsed->as(Entity);
	REQUIRE(reader);
	return *reader;
}

struct stun
{
	using protocol_type = turner::stun;
	static constexpr auto message = turner::stun::binding.request;
	static constexpr auto &transaction_id = stun_id;
	static constexpr auto attribute = turner::stun::username;
	static constexpr auto unknown_attributes = turner::stun::unknown_attributes;
	static constexpr auto xor_address = turner::stun::xor_mapped_address;
};

struct turn
{
	using protocol_type = turner::turn;
	static constexpr auto message = turner::turn::allocate;
	static constexpr auto &transaction_id = stun_id;
	static constexpr auto attribute = turner::turn::username;
	static constexpr auto unknown_attributes = turner::turn::unknown_attributes;
	static constexpr auto xor_address = turner::turn::xor_peer_address;
	static constexpr auto data = turner::turn::data;
	static constexpr size_t largest_length = 0xfffc;
};

struct msturn
{
	using protocol_type = turner::msturn;
	static constexpr auto message = turner::msturn::allocate;
	static constexpr auto &transaction_id = msturn_id;
	static constexpr auto attribute = turner::msturn::realm;
	static constexpr auto unknown_attributes = turner::msturn::unknown_attributes;
	static constexpr auto xor_address = turner::msturn::xor_mapped_address;
	static constexpr auto data = turner::msturn::data;
	static constexpr size_t largest_length = 0xffff;
};

TEMPLATE_TEST_CASE("writer/write", "", stun, turn, msturn)
{
	using Protocol = typename TestType::protocol_type;
	using writer_type = writer<TestType::message>;
	constexpr auto attribute = TestType::attribute;
	constexpr auto attributes_offset = turner::__wire::attributes_offset<Protocol>;

	auto buffer = make_buffer();
	std::vector<std::byte> copy;

	SECTION("header")
	{
		auto exact = make_array<attributes_offset>();
		const auto message = writer_type{exact, TestType::transaction_id}.finish();
		REQUIRE(message);
		CHECK(message->size() == attributes_offset);

		const auto request = parse_as<Protocol, TestType::message>(*message, copy);
		CHECK(request.transaction_id() == TestType::transaction_id);
		CHECK(std::ranges::empty(request.wire_attributes()));
	}

	SECTION("repeated attribute")
	{
		const auto message = writer_type{buffer, TestType::transaction_id}
			.write(attribute, "a")
			.write(attribute, "bc")
			.finish();
		REQUIRE(message);

		const auto request = parse_as<Protocol, TestType::message>(*message, copy);
		const auto values = request.read_each(attribute);
		const std::vector results(values.begin(), values.end());
		REQUIRE(results.size() == 2);
		CHECK(results[0] == "a");
		CHECK(results[1] == "bc");
	}

	SECTION("xor key")
	{
		const auto message = writer_type{buffer, TestType::transaction_id}
			.write(TestType::xor_address, server_address)
			.finish();
		REQUIRE(message);
		CHECK(std::ranges::equal(message->subspan(attributes_offset + 6, 2), turner_test::bytes("2c84")));

		const auto request = parse_as<Protocol, TestType::message>(*message, copy);
		CHECK(request.read(TestType::xor_address) == server_address);
	}

	SECTION("lvalue with conditional attribute")
	{
		const auto build = [](std::span<std::byte> into, bool with_attribute)
		{
			writer_type request{into, TestType::transaction_id};
			if (with_attribute)
			{
				request.write(TestType::attribute, "a");
			}
			return std::move(request).finish();
		};

		const auto without = build(buffer, false);
		REQUIRE(without);
		CHECK(without->size() == attributes_offset);

		const auto with = build(buffer, true);
		REQUIRE(with);
		const auto request = parse_as<Protocol, TestType::message>(*with, copy);
		CHECK(request.read(attribute) == "a");
	}
}

TEMPLATE_TEST_CASE("writer/failure", "", stun, turn, msturn)
{
	using Protocol = typename TestType::protocol_type;
	using writer_type = writer<TestType::message>;
	constexpr auto attribute = TestType::attribute;
	constexpr auto attributes_offset = turner::__wire::attributes_offset<Protocol>;
	const std::string too_long(decltype(attribute)::value_type::max_size + 1, 'u');

	SECTION("empty buffer")
	{
		std::vector<std::byte> buffer;
		CHECK(writer_type{buffer, TestType::transaction_id}
			.write(attribute, "a")
			.finish()
			==
			failed(turner::errc::buffer_too_small)
		);
	}

	SECTION("header")
	{
		auto buffer = make_array<attributes_offset - 1>();
		writer_type request{buffer, TestType::transaction_id};
		request.write(attribute, too_long);
		request.write(attribute, "a");
		CHECK(std::move(request).finish() == failed(turner::errc::buffer_too_small));
	}

	SECTION("attribute")
	{
		const auto build = [](std::span<std::byte> buffer)
		{
			return writer_type{buffer, TestType::transaction_id}
				.write(TestType::attribute, "abcd")
				.finish();
		};

		auto exact = make_array<attributes_offset + 8>();
		const auto message = build(exact);
		REQUIRE(message);
		CHECK(message->size() == attributes_offset + 8);

		auto short_value = make_array<attributes_offset + 7>();
		CHECK(build(short_value) == failed(turner::errc::buffer_too_small));

		auto short_header = make_array<attributes_offset + 3>();
		CHECK(build(short_header) == failed(turner::errc::buffer_too_small));
	}

	SECTION("above max_size")
	{
		auto buffer = make_buffer(2 * too_long.size());
		CHECK(writer_type{buffer, TestType::transaction_id}
			.write(attribute, too_long)
			.finish()
			==
			failed(turner::errc::unexpected_attribute_length)
		);
	}

	SECTION("below min_size")
	{
		auto buffer = make_buffer();
		CHECK(writer_type{buffer, TestType::transaction_id}
			.write(TestType::unknown_attributes, turner::attribute_type_list{})
			.finish()
			==
			failed(turner::errc::unexpected_attribute_length)
		);
	}

	SECTION("first failure is kept")
	{
		auto small = make_array<attributes_offset>();
		CHECK(writer_type{small, TestType::transaction_id}
			.write(attribute, "x")
			.write(attribute, too_long)
			.finish()
			==
			failed(turner::errc::buffer_too_small)
		);
		CHECK(writer_type{small, TestType::transaction_id}
			.write(attribute, too_long)
			.write(attribute, "x")
			.finish()
			==
			failed(turner::errc::unexpected_attribute_length)
		);

		auto large = make_buffer(2 * too_long.size());
		CHECK(writer_type{large, TestType::transaction_id}
			.write(attribute, too_long)
			.write(attribute, "x")
			.finish()
			==
			failed(turner::errc::unexpected_attribute_length)
		);

		auto fits_smaller = make_array<attributes_offset + 8>();
		CHECK(writer_type{fits_smaller, TestType::transaction_id}
			.write(attribute, "hello")
			.write(attribute, "")
			.finish()
			==
			failed(turner::errc::buffer_too_small)
		);
		CHECK(fits_smaller[attributes_offset] == filler);
	}
}

TEMPLATE_TEST_CASE("writer/message size limit", "", turn, msturn)
{
	using Protocol = typename TestType::protocol_type;
	using writer_type = writer<TestType::message>;
	constexpr auto attributes_offset = turner::__wire::attributes_offset<Protocol>;
	constexpr size_t largest_data = TestType::largest_length - (attributes_offset - Protocol::header_size) - 4;

	auto buffer = make_buffer(Protocol::header_size + 0xffff + 16);
	std::vector<std::byte> copy;

	SECTION("largest message")
	{
		const std::vector<std::byte> data(largest_data);
		const auto message = writer_type{buffer, TestType::transaction_id}
			.write(TestType::data, data)
			.finish();
		REQUIRE(message);
		CHECK(message->size() == Protocol::header_size + TestType::largest_length);

		const auto request = parse_as<Protocol, TestType::message>(*message, copy);
		const auto value = request.read(TestType::data);
		REQUIRE(value);
		CHECK(value->size() == largest_data);
	}

	SECTION("byte beyond largest message")
	{
		const std::vector<std::byte> data(largest_data + 1);
		CHECK(writer_type{buffer, TestType::transaction_id}
			.write(TestType::data, data)
			.finish()
			==
			failed(turner::errc::buffer_too_small)
		);
	}

	SECTION("largest value")
	{
		const std::vector<std::byte> data(0xffff);
		CHECK(writer_type{buffer, TestType::transaction_id}
			.write(TestType::data, data)
			.finish()
			==
			failed(turner::errc::buffer_too_small)
		);
	}
}

TEST_CASE("writer/vectors")
{
	auto buffer = make_buffer();
	std::vector<std::byte> copy;

	SECTION("stun: binding indication")
	{
		const auto message = writer<turner::stun::binding.indication>{buffer, stun_id}.finish();
		CHECK(hex(message) == hex(turner_test::stun_message::binding_indication));
	}

	SECTION("turn: allocate request")
	{
		const auto message = writer<turner::turn::allocate>{buffer, stun_id}
			.write(turner::turn::requested_transport, turner::transport_protocol::udp)
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::allocate_request));

		REQUIRE(message);
		const auto request = parse_as<turner::turn, turner::turn::allocate>(*message, copy);
		CHECK(request.transaction_id() == stun_id);
		CHECK(request.read(turner::turn::requested_transport) == turner::transport_protocol::udp);
	}

	SECTION("turn: allocate request with flag")
	{
		const auto message = writer<turner::turn::allocate>{buffer, stun_id}
			.write(turner::turn::dont_fragment)
			.write(turner::turn::requested_transport, turner::transport_protocol::udp)
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::allocate_request_dont_fragment));

		REQUIRE(message);
		const auto request = parse_as<turner::turn, turner::turn::allocate>(*message, copy);
		CHECK(request.read(turner::turn::dont_fragment));
	}

	SECTION("turn: refresh request")
	{
		const auto message = writer<turner::turn::refresh>{buffer, stun_id}
			.write(turner::turn::lifetime, 0s)
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::refresh_request));

		REQUIRE(message);
		const auto request = parse_as<turner::turn, turner::turn::refresh>(*message, copy);
		CHECK(request.read(turner::turn::lifetime) == 0s);
	}

	SECTION("turn: send indication")
	{
		const auto message = writer<turner::turn::send_indication>{buffer, stun_id}
			.write(turner::turn::xor_peer_address, peer_address)
			.write(turner::turn::data, as_bytes("hello"))
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::send_indication));

		REQUIRE(message);
		const auto indication = parse_as<turner::turn, turner::turn::send_indication>(*message, copy);
		CHECK(indication.read(turner::turn::xor_peer_address) == peer_address);
		const auto data = indication.read(turner::turn::data);
		REQUIRE(data);
		CHECK(std::ranges::equal(*data, as_bytes("hello")));
	}

	SECTION("turn: data indication")
	{
		const auto message = writer<turner::turn::data_indication>{buffer, stun_id}
			.write(turner::turn::xor_peer_address, peer_address)
			.write(turner::turn::data, as_bytes("hello"))
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::data_indication));
	}

	SECTION("turn: create permission request")
	{
		const auto message = writer<turner::turn::create_permission>{buffer, stun_id}
			.write(turner::turn::xor_peer_address, peer_address)
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::create_permission_request));
	}

	SECTION("turn: channel bind request")
	{
		const auto message = writer<turner::turn::channel_bind>{buffer, stun_id}
			.write(turner::turn::channel_number, uint16_t{0x4000})
			.write(turner::turn::xor_peer_address, peer_address)
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::channel_bind_request));

		REQUIRE(message);
		const auto request = parse_as<turner::turn, turner::turn::channel_bind>(*message, copy);
		CHECK(request.read(turner::turn::channel_number) == 0x4000);
		CHECK(request.read(turner::turn::xor_peer_address) == peer_address);
	}

	SECTION("msturn: allocate request")
	{
		const auto message = writer<turner::msturn::allocate>{buffer, msturn_id}
			.write(turner::msturn::ms_version, turner::msturn::protocol_version::v4)
			.write(
				turner::msturn::ms_service_quality,
				{
					.stream = turner::msturn::stream_type::audio,
					.quality = turner::msturn::delivery::best_effort,
				}
			)
			.write(turner::msturn::username, "alice")
			.finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::allocate_request));

		REQUIRE(message);
		const auto request = parse_as<turner::msturn, turner::msturn::allocate>(*message, copy);
		CHECK(request.transaction_id() == msturn_id);
		CHECK(request.read(turner::msturn::ms_version) == turner::msturn::protocol_version::v4);
		CHECK(request.read(turner::msturn::username) == "alice");
	}

	SECTION("msturn: allocate request with empty attributes")
	{
		const auto message = writer<turner::msturn::allocate>{buffer, msturn_id}
			.write(turner::msturn::username, "")
			.write(turner::msturn::ms_version, turner::msturn::protocol_version::v4)
			.write(turner::msturn::realm, "")
			.finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::allocate_request_empty_attributes));
	}

	SECTION("msturn: send request")
	{
		const auto message = writer<turner::msturn::send_request>{buffer, msturn_id}
			.write(turner::msturn::ms_version, turner::msturn::protocol_version::v4)
			.write(turner::msturn::data, as_bytes("abc"))
			.write(turner::msturn::destination_address, server_address)
			.write(turner::msturn::username, "alice")
			.finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::send_request));

		REQUIRE(message);
		const auto request = parse_as<turner::msturn, turner::msturn::send_request>(*message, copy);
		CHECK(request.read(turner::msturn::destination_address) == server_address);
	}

	SECTION("msturn: data indication")
	{
		const auto message = writer<turner::msturn::data_indication>{buffer, msturn_id}
			.write(turner::msturn::remote_address, server_address)
			.write(turner::msturn::data, as_bytes("abc"))
			.finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::data_indication));
	}

	SECTION("msturn: set active destination request")
	{
		const auto message = writer<turner::msturn::set_active_destination>{buffer, msturn_id}
			.write(turner::msturn::destination_address, server_address)
			.finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::set_active_destination_request));
	}
}

TEST_CASE("writer/protocol specific")
{
	SECTION("stun: padding is zeroed")
	{
		auto buffer = make_buffer();
		const auto message = writer<turner::stun::binding.request>{buffer, stun_id}
			.write(turner::stun::software, "hello")
			.write(turner::stun::software, "abc")
			.write(turner::stun::software, "abcd")
			.finish();
		CHECK(hex(message)
			==
			"0001001c" "2112a442" "0102030405060708090a0b0c"
			"80220005" "68656c6c" "6f000000"
			"80220003" "61626300"
			"80220004" "61626364"
		);
		CHECK(buffer[48] == filler);
	}

	SECTION("stun: padding does not fit")
	{
		auto buffer = make_array<29>();
		CHECK(writer<turner::stun::binding.request>{buffer, stun_id}
			.write(turner::stun::software, "hello")
			.finish()
			==
			failed(turner::errc::buffer_too_small)
		);
	}

	SECTION("msturn: IPv4 below min_size of IPv6-only endpoint")
	{
		auto buffer = make_buffer();
		CHECK(writer<turner::msturn::allocate>{buffer, msturn_id}
			.write(turner::msturn::ms_alternate_mapped_address, server_address)
			.finish()
			==
			failed(turner::errc::unexpected_attribute_length)
		);
	}
}

} // namespace
