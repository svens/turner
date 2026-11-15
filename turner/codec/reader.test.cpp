#include <turner/codec/test.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{

template <typename Protocol, auto Entity>
concept can_as = requires(const turner::parsed_message<Protocol> &message)
{
	message.as(Entity);
};

static_assert(can_as<turner::stun, turner::stun::binding.request>);
static_assert(can_as<turner::stun, turner::stun::binding.indication>);
static_assert(can_as<turner::turn, turner::turn::allocate>);
static_assert(can_as<turner::turn, turner::turn::allocate.request>);
static_assert(can_as<turner::turn, turner::turn::allocate.success>);
static_assert(can_as<turner::turn, turner::turn::send_indication>);
static_assert(can_as<turner::turn, turner::stun::binding.request>);
static_assert(can_as<turner::msturn, turner::msturn::send_request>);
static_assert(can_as<turner::msturn, turner::msturn::data_indication>);
static_assert(!can_as<turner::stun, turner::stun::binding>);
static_assert(!can_as<turner::turn, turner::turn::binding>);
static_assert(!can_as<turner::stun, turner::turn::allocate>);
static_assert(!can_as<turner::turn, turner::msturn::allocate>);
static_assert(!can_as<turner::msturn, turner::turn::allocate>);
static_assert(!can_as<turner::msturn, turner::stun::binding.request>);

template <auto Entity>
using reader = turner::message_reader<Entity>;

static_assert(std::same_as<reader<turner::turn::allocate>, reader<turner::turn::allocate.request>>);
static_assert(!std::same_as<reader<turner::turn::allocate>, reader<turner::turn::allocate.success>>);
static_assert(std::same_as<reader<turner::turn::binding.request>, reader<turner::stun::binding.request>>);
static_assert(std::same_as<reader<turner::turn::binding.request>::protocol_type, turner::stun>);
static_assert(std::same_as<reader<turner::turn::allocate>::protocol_type, turner::turn>);
static_assert(
	std::same_as<
		decltype(std::declval<turner::parsed_message<turner::turn>>().as(turner::turn::allocate)),
		std::optional<reader<turner::turn::allocate>>
	>
);

static_assert(!std::is_default_constructible_v<turner::parsed_message<turner::turn>>);
static_assert(!std::constructible_from<turner::parsed_message<turner::turn>, std::span<std::byte>>);
static_assert(!std::constructible_from<turner::parsed_message<turner::turn>, turner::parsed_message<turner::stun>>);
static_assert(std::is_trivially_copy_constructible_v<turner::parsed_message<turner::turn>>);
static_assert(!std::is_default_constructible_v<reader<turner::turn::allocate>>);
static_assert(!std::constructible_from<reader<turner::turn::allocate>, std::span<std::byte>>);

struct sample
{
	std::string_view name;
	std::span<const std::byte> bytes;
	uint16_t type;
};

struct malformed
{
	std::string_view name;
	std::span<const std::byte> bytes;
	turner::errc error;
};

#define __turner_test_sample(Namespace, Name, Type) \
	sample{#Namespace "/" #Name, turner_test::Namespace::Name, Type}

#define __turner_test_malformed(Name, Error) \
	malformed{#Name, Name, turner::errc::Error}

struct stun
{
	using protocol_type = turner::stun;

	static constexpr sample samples[] =
	{
		__turner_test_sample(rfc5769, request, 0x0001),
		__turner_test_sample(rfc5769, ipv4_response, 0x0101),
		__turner_test_sample(rfc5769, ipv6_response, 0x0101),
		__turner_test_sample(rfc5769, long_term_request, 0x0001),
		__turner_test_sample(stun_message, binding_indication, 0x0011),
		__turner_test_sample(stun_message, binding_error, 0x0111),
		__turner_test_sample(turn_message, allocate_request, 0x0003),
		__turner_test_sample(turn_message, allocate_request_dont_fragment, 0x0003),
		__turner_test_sample(turn_message, allocate_success, 0x0103),
		__turner_test_sample(turn_message, allocate_error, 0x0113),
		__turner_test_sample(turn_message, refresh_request, 0x0004),
		__turner_test_sample(turn_message, refresh_success, 0x0104),
		__turner_test_sample(turn_message, refresh_error, 0x0114),
		__turner_test_sample(turn_message, send_indication, 0x0016),
		__turner_test_sample(turn_message, data_indication, 0x0017),
		__turner_test_sample(turn_message, create_permission_request, 0x0008),
		__turner_test_sample(turn_message, create_permission_success, 0x0108),
		__turner_test_sample(turn_message, create_permission_error, 0x0118),
		__turner_test_sample(turn_message, channel_bind_request, 0x0009),
		__turner_test_sample(turn_message, channel_bind_success, 0x0109),
		__turner_test_sample(turn_message, channel_bind_error, 0x0119),
	};

	static constexpr auto length_misaligned_by_1 = turner_test::bytes(
		"00030009" "2112a442" "01020304" "05060708" "090a0b0c"
		"00190004" "11000000"
	);

	static constexpr auto length_misaligned_by_2 = turner_test::bytes(
		"0003000a" "2112a442" "01020304" "05060708" "090a0b0c"
		"00190004" "11000000"
	);

	static constexpr auto length_misaligned_by_3 = turner_test::bytes(
		"0003000b" "2112a442" "01020304" "05060708" "090a0b0c"
		"00190004" "11000000"
	);

	static constexpr auto length_cuts_attribute = turner_test::bytes(
		"00030004" "2112a442" "01020304" "05060708" "090a0b0c"
		"00190004" "11000000"
	);

	static constexpr auto attribute_exceeds_message = turner_test::bytes(
		"00030008" "2112a442" "01020304" "05060708" "090a0b0c"
		"00190008" "11000000"
	);

	static constexpr auto attribute_padding_exceeds_message = turner_test::bytes(
		"00030008" "2112a442" "01020304" "05060708" "090a0b0c"
		"00190005" "11000000"
	);

	static constexpr auto attribute_max_length = turner_test::bytes(
		"00030008" "2112a442" "01020304" "05060708" "090a0b0c"
		"0019ffff" "11000000"
	);

	static constexpr malformed malformed_messages[] =
	{
		__turner_test_malformed(length_misaligned_by_1, unexpected_message_length),
		__turner_test_malformed(length_misaligned_by_2, unexpected_message_length),
		__turner_test_malformed(length_misaligned_by_3, unexpected_message_length),
		__turner_test_malformed(length_cuts_attribute, unexpected_attribute_length),
		__turner_test_malformed(attribute_exceeds_message, unexpected_attribute_length),
		__turner_test_malformed(attribute_padding_exceeds_message, unexpected_attribute_length),
		__turner_test_malformed(attribute_max_length, unexpected_attribute_length),
	};
};

struct turn: stun
{
	using protocol_type = turner::turn;
};

struct msturn
{
	using protocol_type = turner::msturn;

	static constexpr sample samples[] =
	{
		__turner_test_sample(msturn_message, allocate_request, 0x0003),
		__turner_test_sample(msturn_message, allocate_request_empty_attributes, 0x0003),
		__turner_test_sample(msturn_message, allocate_success, 0x0103),
		__turner_test_sample(msturn_message, allocate_error, 0x0113),
		__turner_test_sample(msturn_message, send_request, 0x0004),
		__turner_test_sample(msturn_message, data_indication, 0x0115),
		__turner_test_sample(msturn_message, set_active_destination_request, 0x0006),
		__turner_test_sample(msturn_message, set_active_destination_success, 0x0106),
		__turner_test_sample(msturn_message, set_active_destination_error, 0x0116),
	};

	static constexpr auto length_zero = turner_test::bytes(
		"00060000" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
	);

	static constexpr auto length_below_cookie = turner_test::bytes(
		"00060007" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
	);

	static constexpr auto attribute_header_1_byte = turner_test::bytes(
		"00060009" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
		"00"
	);

	static constexpr auto attribute_header_2_bytes = turner_test::bytes(
		"0006000a" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
		"0011"
	);

	static constexpr auto attribute_header_3_bytes = turner_test::bytes(
		"0006000b" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
		"001100"
	);

	static constexpr auto length_cuts_attribute = turner_test::bytes(
		"00060013" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
		"00110008" "00010d96" "c0000201"
	);

	static constexpr auto attribute_exceeds_message = turner_test::bytes(
		"00060014" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
		"00110009" "00010d96" "c0000201"
	);

	static constexpr auto attribute_max_length = turner_test::bytes(
		"00060014" "2112a442" "01020304" "05060708" "090a0b0c"
		"000f0004" "72c64bc6"
		"0011ffff" "00010d96" "c0000201"
	);

	static constexpr malformed malformed_messages[] =
	{
		__turner_test_malformed(length_zero, unexpected_message_length),
		__turner_test_malformed(length_below_cookie, unexpected_message_length),
		__turner_test_malformed(attribute_header_1_byte, unexpected_attribute_length),
		__turner_test_malformed(attribute_header_2_bytes, unexpected_attribute_length),
		__turner_test_malformed(attribute_header_3_bytes, unexpected_attribute_length),
		__turner_test_malformed(length_cuts_attribute, unexpected_attribute_length),
		__turner_test_malformed(attribute_exceeds_message, unexpected_attribute_length),
		__turner_test_malformed(attribute_max_length, unexpected_attribute_length),
	};
};

#undef __turner_test_malformed
#undef __turner_test_sample

constexpr size_t trailing_size = 7;

std::vector<std::byte> make_buffer (std::span<const std::byte> bytes, size_t trailing = 0)
{
	std::vector<std::byte> buffer{bytes.begin(), bytes.end()};
	buffer.resize(buffer.size() + trailing, std::byte{0xa5});
	return buffer;
}

TEMPLATE_TEST_CASE("reader/parse", "", stun, turn, msturn)
{
	using Protocol = typename TestType::protocol_type;

	const size_t first_bytes_size = (std::max)(
		Protocol::header_size,
		Protocol::cookie_offset + Protocol::cookie.size()
	);

	SECTION("valid")
	{
		const auto s = GENERATE(from_range(TestType::samples));
		CAPTURE(s.name);
		auto buffer = make_buffer(s.bytes);

		const auto message = Protocol::parse(buffer, buffer.size());
		REQUIRE(message);
		CHECK(message->size() == s.bytes.size());
		CHECK(message->type() == s.type);

		const auto &id = message->transaction_id();
		CHECK(static_cast<const void *>(id.data()) == buffer.data() + Protocol::transaction_id_offset);
		CHECK(std::ranges::equal(id, s.bytes.subspan(Protocol::transaction_id_offset, id.size())));
	}

	SECTION("trailing bytes")
	{
		const auto s = GENERATE(from_range(TestType::samples));
		CAPTURE(s.name);
		auto buffer = make_buffer(s.bytes, trailing_size);

		const auto message = Protocol::parse(buffer, buffer.size());
		REQUIRE(message);
		CHECK(message->size() == s.bytes.size());

		const auto without_trailing_bytes = Protocol::parse(buffer, s.bytes.size());
		REQUIRE(without_trailing_bytes);
		CHECK(without_trailing_bytes->size() == s.bytes.size());
	}

	SECTION("incomplete")
	{
		const auto s = GENERATE(from_range(TestType::samples));
		CAPTURE(s.name);

		auto whole = make_buffer(s.bytes, trailing_size);

		for (size_t received = 0; received < s.bytes.size(); ++received)
		{
			CAPTURE(received);

			auto prefix = make_buffer(s.bytes.first(received));
			const auto message = Protocol::parse(prefix, prefix.size());
			REQUIRE_FALSE(message);
			CHECK(message.error() == turner::errc::incomplete);

			const auto in_larger_buffer = Protocol::parse(whole, received);
			REQUIRE_FALSE(in_larger_buffer);
			CHECK(in_larger_buffer.error() == turner::errc::incomplete);
		}
	}

	SECTION("reserved type bits")
	{
		const auto s = GENERATE(from_range(TestType::samples));
		const auto bits = GENERATE(0x40, 0x80, 0xc0);
		CAPTURE(s.name, bits);
		auto buffer = make_buffer(s.bytes);
		buffer[0] |= static_cast<std::byte>(bits);

		const auto message = Protocol::parse(buffer, buffer.size());
		REQUIRE_FALSE(message);
		CHECK(message.error() == turner::errc::unexpected_message_type);

		const auto first_bytes = Protocol::parse(buffer, first_bytes_size);
		REQUIRE_FALSE(first_bytes);
		CHECK(first_bytes.error() == turner::errc::unexpected_message_type);
	}

	SECTION("magic cookie")
	{
		const auto s = GENERATE(from_range(TestType::samples));
		CAPTURE(s.name);

		for (size_t i = 0; i < Protocol::cookie.size(); ++i)
		{
			CAPTURE(i);
			auto buffer = make_buffer(s.bytes);
			buffer[Protocol::cookie_offset + i] ^= std::byte{0x01};

			const auto message = Protocol::parse(buffer, buffer.size());
			REQUIRE_FALSE(message);
			CHECK(message.error() == turner::errc::invalid_magic_cookie);

			const auto first_bytes = Protocol::parse(buffer, first_bytes_size);
			REQUIRE_FALSE(first_bytes);
			CHECK(first_bytes.error() == turner::errc::invalid_magic_cookie);
		}
	}

	SECTION("malformed")
	{
		const auto m = GENERATE(from_range(TestType::malformed_messages));
		CAPTURE(m.name);
		auto buffer = make_buffer(m.bytes);

		const auto message = Protocol::parse(buffer, buffer.size());
		REQUIRE_FALSE(message);
		CHECK(message.error() == m.error);

		auto with_trailing_bytes = make_buffer(m.bytes, trailing_size);
		const auto same = Protocol::parse(with_trailing_bytes, with_trailing_bytes.size());
		REQUIRE_FALSE(same);
		CHECK(same.error() == m.error);
	}
}

TEST_CASE("reader/parse/other protocol")
{
	SECTION("msturn message with unaligned attributes as stun")
	{
		auto buffer = make_buffer(turner_test::msturn_message::allocate_request);
		const auto message = turner::stun::parse(buffer, buffer.size());
		REQUIRE_FALSE(message);
		CHECK(message.error() == turner::errc::unexpected_message_length);
	}

	SECTION("msturn message with aligned attributes as stun")
	{
		const auto &bytes = turner_test::msturn_message::set_active_destination_request;
		auto buffer = make_buffer(bytes);
		const auto message = turner::stun::parse(buffer, buffer.size());
		REQUIRE(message);
		CHECK(message->size() == bytes.size());
	}

	SECTION("stun message as msturn")
	{
		auto buffer = make_buffer(turner_test::rfc5769::request);
		const auto message = turner::msturn::parse(buffer, buffer.size());
		REQUIRE_FALSE(message);
		CHECK(message.error() == turner::errc::invalid_magic_cookie);
	}
}

TEST_CASE("reader/as")
{
	SECTION("turn")
	{
		auto buffer = make_buffer(turner_test::turn_message::allocate_request);
		const auto message = turner::turn::parse(buffer, buffer.size());
		REQUIRE(message);

		const auto request = message->as(turner::turn::allocate);
		REQUIRE(request);
		CHECK(request->transaction_id() == message->transaction_id());
		CHECK(&request->transaction_id() == &message->transaction_id());

		CHECK(message->as(turner::turn::allocate.request));
		CHECK_FALSE(message->as(turner::turn::allocate.success));
		CHECK_FALSE(message->as(turner::turn::allocate.error));
		CHECK_FALSE(message->as(turner::turn::refresh));
		CHECK_FALSE(message->as(turner::turn::send_indication));
		CHECK_FALSE(message->as(turner::stun::binding.request));
	}

	SECTION("type differs in high bits")
	{
		const auto high_byte = GENERATE(0x10, 0x20, 0x3f);
		CAPTURE(high_byte);
		auto buffer = make_buffer(turner_test::turn_message::allocate_request);
		buffer[0] = static_cast<std::byte>(high_byte);
		const auto message = turner::turn::parse(buffer, buffer.size());
		REQUIRE(message);
		CHECK(message->type() == ((high_byte << 8) | 0x03));
		CHECK_FALSE(message->as(turner::turn::allocate));
	}

	SECTION("stun message in turn")
	{
		auto buffer = make_buffer(turner_test::rfc5769::request);
		const auto message = turner::turn::parse(buffer, buffer.size());
		REQUIRE(message);

		const auto request = message->as(turner::stun::binding.request);
		REQUIRE(request);
		CHECK(request->transaction_id() == message->transaction_id());

		CHECK(message->as(turner::turn::binding.request));
		CHECK_FALSE(message->as(turner::turn::binding.indication));
		CHECK_FALSE(message->as(turner::turn::binding.success));
		CHECK_FALSE(message->as(turner::turn::allocate));
	}

	SECTION("msturn")
	{
		auto buffer = make_buffer(turner_test::msturn_message::data_indication);
		const auto message = turner::msturn::parse(buffer, buffer.size());
		REQUIRE(message);

		const auto indication = message->as(turner::msturn::data_indication);
		REQUIRE(indication);
		CHECK(indication->transaction_id() == message->transaction_id());

		CHECK_FALSE(message->as(turner::msturn::allocate.error));
		CHECK_FALSE(message->as(turner::msturn::send_request));
	}
}

} // namespace
