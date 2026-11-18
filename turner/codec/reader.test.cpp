#include <turner/codec/test.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol_error.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <turner/codec/writer.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{

using namespace std::chrono_literals;
using turner_test::buffer_of;
using turner_test::failed;
using turner_test::hex;
using turner_test::make_array;
using turner_test::make_buffer;
using turner_test::msturn_id;
using turner_test::parse_as;
using turner_test::parse_in;
using turner_test::peer_address;
using turner_test::server_address;
using turner_test::stun_id;

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

template <auto Entity>
using writer = turner::message_writer<Entity>;

template <typename Reader>
concept can_succeed = requires(Reader &&request)
{
	std::forward<Reader>(request).to_success();
};

template <typename Reader>
concept can_fail = requires(Reader &&request)
{
	std::forward<Reader>(request).to_error(turner::protocol_errc::bad_request);
};

constexpr auto success_only = turner::method<
	turner::stun,
	0x0ff0,
	turner::message_class::request | turner::message_class::success
>;

constexpr auto error_only = turner::method<
	turner::stun,
	0x0ff1,
	turner::message_class::request | turner::message_class::error
>;

static_assert(can_succeed<reader<turner::stun::binding.request>>);
static_assert(can_fail<reader<turner::stun::binding.request>>);
static_assert(can_succeed<reader<turner::msturn::allocate>>);
static_assert(can_fail<reader<turner::msturn::allocate>>);
static_assert(!can_succeed<reader<turner::turn::allocate> &>);
static_assert(!can_fail<reader<turner::turn::allocate> &>);
static_assert(!can_succeed<const reader<turner::turn::allocate> &&>);
static_assert(!can_fail<const reader<turner::turn::allocate> &&>);
static_assert(!can_succeed<reader<turner::stun::binding.indication>>);
static_assert(!can_fail<reader<turner::stun::binding.indication>>);
static_assert(!can_succeed<reader<turner::turn::allocate.success>>);
static_assert(!can_fail<reader<turner::turn::allocate.success>>);
static_assert(!can_succeed<reader<turner::turn::allocate.error>>);
static_assert(!can_fail<reader<turner::turn::allocate.error>>);
static_assert(!can_succeed<reader<turner::msturn::send_request>>);
static_assert(!can_fail<reader<turner::msturn::send_request>>);
static_assert(can_succeed<reader<success_only>>);
static_assert(!can_fail<reader<success_only>>);
static_assert(!can_succeed<reader<error_only>>);
static_assert(can_fail<reader<error_only>>);
static_assert(
	std::same_as<
		decltype(std::declval<reader<turner::turn::allocate>>().to_success()),
		writer<turner::turn::allocate.success>
	>
);
static_assert(
	std::same_as<
		decltype(std::declval<reader<turner::turn::allocate>>().to_error(turner::protocol_errc::bad_request)),
		writer<turner::turn::allocate.error>
	>
);
static_assert(
	std::same_as<
		decltype(std::declval<reader<turner::turn::binding.request>>().to_success()),
		writer<turner::stun::binding.success>
	>
);
static_assert(noexcept(std::declval<reader<turner::turn::allocate>>().to_success()));
static_assert(noexcept(std::declval<reader<turner::turn::allocate>>().to_error(turner::protocol_errc::bad_request)));

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
	static constexpr auto message = turner::stun::binding.request;
	static constexpr auto success = turner::stun::binding.success;
	static constexpr auto error = turner::stun::binding.error;
	static constexpr auto &transaction_id = stun_id;
	static constexpr auto attribute = turner::stun::username;
	static constexpr auto xor_address = turner::stun::xor_mapped_address;

	static constexpr sample samples[] =
	{
		__turner_test_sample(rfc5769, request, 0x0001),
		__turner_test_sample(rfc5769, ipv4_response, 0x0101),
		__turner_test_sample(rfc5769, ipv6_response, 0x0101),
		__turner_test_sample(rfc5769, long_term_request, 0x0001),
		__turner_test_sample(stun_message, binding_indication, 0x0011),
		__turner_test_sample(stun_message, binding_error, 0x0111),
		__turner_test_sample(stun_message, binding_request_fingerprint, 0x0001),
		__turner_test_sample(stun_message, binding_request_software_fingerprint, 0x0001),
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

	static constexpr auto fingerprint_not_last = turner_test::bytes(
		"00010010" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280004" "aa612f2f"
		"80220003" "61626300"
	);

	static constexpr auto fingerprint_twice = turner_test::bytes(
		"00010010" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280004" "aa612f2f"
		"80280004" "aa612f2f"
	);

	static constexpr auto fingerprint_not_last_with_bad_length = turner_test::bytes(
		"0001000c" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280000"
		"80220003" "61626300"
	);

	static constexpr auto fingerprint_length_0 = turner_test::bytes(
		"00010004" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280000"
	);

	static constexpr auto fingerprint_length_3 = turner_test::bytes(
		"00010008" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280003" "5b20f900"
	);

	static constexpr auto fingerprint_length_8 = turner_test::bytes(
		"0001000c" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280008" "5b20f9cc" "00000000"
	);

	static constexpr auto fingerprint_mismatch = turner_test::bytes(
		"00010008" "2112a442" "01020304" "05060708" "090a0b0c"
		"80280004" "5b20f9cd"
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
		__turner_test_malformed(fingerprint_not_last, fingerprint_not_last),
		__turner_test_malformed(fingerprint_twice, fingerprint_not_last),
		__turner_test_malformed(fingerprint_not_last_with_bad_length, fingerprint_not_last),
		__turner_test_malformed(fingerprint_length_0, unexpected_attribute_length),
		__turner_test_malformed(fingerprint_length_3, unexpected_attribute_length),
		__turner_test_malformed(fingerprint_length_8, unexpected_attribute_length),
		__turner_test_malformed(fingerprint_mismatch, fingerprint_mismatch),
	};
};

struct turn: stun
{
	using protocol_type = turner::turn;
	static constexpr auto message = turner::turn::allocate;
	static constexpr auto success = turner::turn::allocate.success;
	static constexpr auto error = turner::turn::allocate.error;
	static constexpr auto xor_address = turner::turn::xor_peer_address;
	static constexpr auto data = turner::turn::data;
	static constexpr size_t largest_length = 0xfffc;
};

struct msturn
{
	using protocol_type = turner::msturn;
	static constexpr auto message = turner::msturn::allocate;
	static constexpr auto success = turner::msturn::allocate.success;
	static constexpr auto error = turner::msturn::allocate.error;
	static constexpr auto &transaction_id = msturn_id;
	static constexpr auto attribute = turner::msturn::realm;
	static constexpr auto xor_address = turner::msturn::xor_mapped_address;
	static constexpr auto data = turner::msturn::data;
	static constexpr size_t largest_length = 0xffff;

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

template <typename Protocol>
std::span<const std::byte> kept_header (std::span<const std::byte> message) noexcept
{
	return message.subspan(4, turner::__wire::attributes_offset<Protocol> - 4);
}

TEMPLATE_TEST_CASE("reader", "", stun, turn, msturn)
{
	using Protocol = typename TestType::protocol_type;
	using writer_type = writer<TestType::message>;
	constexpr auto attribute = TestType::attribute;
	constexpr auto attributes_offset = turner::__wire::attributes_offset<Protocol>;
	constexpr bool has_fingerprint = requires { Protocol::fingerprint; };

	SECTION("parse")
	{
		const size_t first_bytes_size = (std::max)(
			Protocol::header_size,
			Protocol::cookie_offset + Protocol::cookie.size()
		);

		SECTION("valid")
		{
			const auto s = GENERATE(from_range(TestType::samples));
			CAPTURE(s.name);
			auto buffer = buffer_of(s.bytes);

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
			auto buffer = buffer_of(s.bytes, trailing_size);

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

			auto whole = buffer_of(s.bytes, trailing_size);

			for (size_t received = 0; received < s.bytes.size(); ++received)
			{
				CAPTURE(received);

				auto prefix = buffer_of(s.bytes.first(received));
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
			auto buffer = buffer_of(s.bytes);
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
				auto buffer = buffer_of(s.bytes);
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
			auto buffer = buffer_of(m.bytes);

			const auto message = Protocol::parse(buffer, buffer.size());
			REQUIRE_FALSE(message);
			CHECK(message.error() == m.error);

			auto with_trailing_bytes = buffer_of(m.bytes, trailing_size);
			const auto same = Protocol::parse(with_trailing_bytes, with_trailing_bytes.size());
			REQUIRE_FALSE(same);
			CHECK(same.error() == m.error);
		}

		if constexpr (has_fingerprint)
		{
			SECTION("fingerprint covered byte changed")
			{
				const auto offset = GENERATE(size_t{1}, size_t{10}, turner_test::rfc5769::request.size() - 9);
				CAPTURE(offset);
				auto buffer = buffer_of(turner_test::rfc5769::request);
				buffer[offset] ^= std::byte{0x01};

				const auto message = Protocol::parse(buffer, buffer.size());
				REQUIRE_FALSE(message);
				CHECK(message.error() == turner::errc::fingerprint_mismatch);
			}
		}
	}

	auto request_buffer = make_buffer();
	const auto request_message = writer_type{request_buffer, TestType::transaction_id}
		.write(attribute, "abcd")
		.finish();
	REQUIRE(request_message);

	auto header_buffer = make_array<attributes_offset>();
	const auto header_message = writer_type{header_buffer, TestType::transaction_id}.finish();
	REQUIRE(header_message);

	auto buffer = make_buffer();
	std::vector<std::byte> copy;

	SECTION("to_success")
	{
		SECTION("response over request")
		{
			auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
			const auto &transaction_id = request.transaction_id();
			const auto message = std::move(request).to_success()
				.write(attribute, "xy")
				.finish();
			REQUIRE(message);
			CHECK(message->data() == buffer.data());
			CHECK(transaction_id == TestType::transaction_id);
			CHECK(std::ranges::equal(kept_header<Protocol>(*message), kept_header<Protocol>(*request_message)));

			const auto response = parse_as<Protocol, TestType::success>(*message, copy);
			CHECK(response.transaction_id() == TestType::transaction_id);
			const auto values = response.read_each(attribute);
			const std::vector results(values.begin(), values.end());
			REQUIRE(results.size() == 1);
			CHECK(results[0] == "xy");
		}

		SECTION("xor address")
		{
			auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
			const auto &transaction_id = request.transaction_id();
			const auto message = std::move(request).to_success()
				.write(TestType::xor_address, server_address)
				.finish();
			REQUIRE(message);
			CHECK(transaction_id == TestType::transaction_id);

			const auto response = parse_as<Protocol, TestType::success>(*message, copy);
			CHECK(response.read(TestType::xor_address) == server_address);
		}

		SECTION("capacity is whole buffer")
		{
			auto exact = make_array<attributes_offset + 8>();

			auto fits = parse_in<Protocol, TestType::message>(exact, *header_message);
			const auto message = std::move(fits).to_success().write(attribute, "abcd").finish();
			REQUIRE(message);
			CHECK(message->size() == attributes_offset + 8);

			auto too_large = parse_in<Protocol, TestType::message>(exact, *header_message);
			CHECK(std::move(too_large).to_success().write(attribute, "abcde").finish()
				==
				failed(turner::errc::buffer_too_small)
			);
		}

		if constexpr (requires { TestType::data; })
		{
			SECTION("message size limit")
			{
				constexpr size_t largest_data =
					TestType::largest_length - (attributes_offset - Protocol::header_size) - 4;
				auto large = make_buffer(Protocol::header_size + 0xffff + 16);

				const std::vector<std::byte> largest(largest_data);
				auto fits = parse_in<Protocol, TestType::message>(large, *header_message);
				const auto message = std::move(fits).to_success().write(TestType::data, largest).finish();
				REQUIRE(message);
				CHECK(message->size() == Protocol::header_size + TestType::largest_length);

				const std::vector<std::byte> too_large(largest_data + 1);
				auto beyond = parse_in<Protocol, TestType::message>(large, *header_message);
				CHECK(std::move(beyond).to_success().write(TestType::data, too_large).finish()
					==
					failed(turner::errc::buffer_too_small)
				);
			}
		}

		if constexpr (has_fingerprint)
		{
			SECTION("fingerprint")
			{
				auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
				const auto message = std::move(request).to_success()
					.write(attribute, "xy")
					.finish(Protocol::fingerprint);
				REQUIRE(message);

				const auto response = parse_as<Protocol, TestType::success>(*message, copy);
				CHECK(response.read(attribute) == "xy");
			}
		}
	}

	SECTION("to_error")
	{
		SECTION("error code is first attribute")
		{
			auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
			const auto &transaction_id = request.transaction_id();
			const auto message = std::move(request).to_error(turner::protocol_errc::unauthorized)
				.write(attribute, "xy")
				.finish();
			REQUIRE(message);
			CHECK(transaction_id == TestType::transaction_id);
			CHECK(std::ranges::equal(kept_header<Protocol>(*message), kept_header<Protocol>(*request_message)));

			const auto response = parse_as<Protocol, TestType::error>(*message, copy);
			const auto attributes = response.wire_attributes();
			REQUIRE(std::ranges::distance(attributes) == 2);
			CHECK((*attributes.begin()).type == decltype(Protocol::error_code)::type);
			const auto error = response.read(Protocol::error_code);
			REQUIRE(error);
			CHECK(*error == turner::protocol_errc::unauthorized);
			CHECK(error->reason.starts_with("Unauthorized"));
			CHECK(response.read(attribute) == "xy");
		}

		SECTION("custom reason")
		{
			auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
			const auto message = std::move(request).to_error({turner::protocol_errc::stale_nonce, "abc"}).finish();
			REQUIRE(message);

			const auto response = parse_as<Protocol, TestType::error>(*message, copy);
			const auto error = response.read(Protocol::error_code);
			REQUIRE(error);
			CHECK(*error == turner::protocol_errc::stale_nonce);
			CHECK(error->reason.starts_with("abc"));
		}

		SECTION("reason too long")
		{
			const std::string too_long(decltype(Protocol::error_code)::value_type::max_size - 3, 'r');
			auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
			CHECK(std::move(request).to_error({turner::protocol_errc::bad_request, too_long}).finish()
				==
				failed(turner::errc::unexpected_attribute_length)
			);
		}

		SECTION("error code does not fit")
		{
			auto exact = make_array<attributes_offset>();
			auto request = parse_in<Protocol, TestType::message>(exact, *header_message);
			CHECK(std::move(request).to_error(turner::protocol_errc::unauthorized).finish()
				==
				failed(turner::errc::buffer_too_small)
			);
		}

		if constexpr (has_fingerprint)
		{
			SECTION("fingerprint")
			{
				auto request = parse_in<Protocol, TestType::message>(buffer, *request_message);
				const auto message = std::move(request).to_error(turner::protocol_errc::unauthorized)
					.finish(Protocol::fingerprint);
				REQUIRE(message);

				const auto response = parse_as<Protocol, TestType::error>(*message, copy);
				CHECK(response.read(Protocol::error_code) == turner::protocol_errc::unauthorized);
			}
		}
	}
}

TEST_CASE("reader/protocol specific")
{
	SECTION("as: turn")
	{
		auto buffer = buffer_of(turner_test::turn_message::allocate_request);
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

	SECTION("as: type differs in high bits")
	{
		const auto high_byte = GENERATE(0x10, 0x20, 0x3f);
		CAPTURE(high_byte);
		auto buffer = buffer_of(turner_test::turn_message::allocate_request);
		buffer[0] = static_cast<std::byte>(high_byte);
		const auto message = turner::turn::parse(buffer, buffer.size());
		REQUIRE(message);
		CHECK(message->type() == ((high_byte << 8) | 0x03));
		CHECK_FALSE(message->as(turner::turn::allocate));
	}

	SECTION("as: stun message in turn")
	{
		auto buffer = buffer_of(turner_test::rfc5769::request);
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

	SECTION("as: msturn")
	{
		auto buffer = buffer_of(turner_test::msturn_message::data_indication);
		const auto message = turner::msturn::parse(buffer, buffer.size());
		REQUIRE(message);

		const auto indication = message->as(turner::msturn::data_indication);
		REQUIRE(indication);
		CHECK(indication->transaction_id() == message->transaction_id());

		CHECK_FALSE(message->as(turner::msturn::allocate.error));
		CHECK_FALSE(message->as(turner::msturn::send_request));
	}

	SECTION("parse: msturn message with unaligned attributes as stun")
	{
		auto buffer = buffer_of(turner_test::msturn_message::allocate_request);
		const auto message = turner::stun::parse(buffer, buffer.size());
		REQUIRE_FALSE(message);
		CHECK(message.error() == turner::errc::unexpected_message_length);
	}

	SECTION("parse: msturn message with aligned attributes as stun")
	{
		const auto &bytes = turner_test::msturn_message::set_active_destination_request;
		auto buffer = buffer_of(bytes);
		const auto message = turner::stun::parse(buffer, buffer.size());
		REQUIRE(message);
		CHECK(message->size() == bytes.size());
	}

	SECTION("parse: stun message as msturn")
	{
		auto buffer = buffer_of(turner_test::rfc5769::request);
		const auto message = turner::msturn::parse(buffer, buffer.size());
		REQUIRE_FALSE(message);
		CHECK(message.error() == turner::errc::invalid_magic_cookie);
	}

	SECTION("parse: msturn 0x8028 is plain attribute")
	{
		auto buffer = buffer_of(turner_test::msturn_message::set_active_destination_request_0x8028);
		const auto message = turner::msturn::parse(buffer, buffer.size());
		REQUIRE(message);
		const auto request = message->as(turner::msturn::set_active_destination);
		REQUIRE(request);
		const auto attributes = request->wire_attributes();
		REQUIRE(std::ranges::distance(attributes) == 1);
		CHECK((*attributes.begin()).type == 0x8028);
	}

	SECTION("to_success: stun binding")
	{
		auto buffer = make_buffer();
		auto request_buffer = make_buffer();
		const auto request_message = writer<turner::stun::binding.request>{request_buffer, stun_id}
			.write(turner::stun::software, "hello")
			.finish();
		REQUIRE(request_message);
		auto request = parse_in<turner::stun, turner::stun::binding.request>(buffer, *request_message);
		const auto message = std::move(request).to_success()
			.write(turner::stun::xor_mapped_address, server_address)
			.finish();
		CHECK(hex(message)
			==
			"0101000c" "2112a442" "0102030405060708090a0b0c"
			"00200008" "00012c84" "e112a643"
		);
	}

	SECTION("to_error: stun binding")
	{
		auto buffer = make_buffer();
		auto request_buffer = make_buffer();
		const auto request_message = writer<turner::stun::binding.request>{request_buffer, stun_id}
			.write(turner::stun::software, "hello")
			.finish();
		REQUIRE(request_message);
		auto request = parse_in<turner::stun, turner::stun::binding.request>(buffer, *request_message);
		const auto message = std::move(request).to_error(turner::protocol_errc::unauthorized).finish();
		CHECK(hex(message) == hex(turner_test::stun_message::binding_error));
	}

	SECTION("to_error: stun reason padding is zeroed")
	{
		auto buffer = make_buffer();
		auto request_buffer = make_buffer();
		const auto request_message = writer<turner::stun::binding.request>{request_buffer, stun_id}.finish();
		REQUIRE(request_message);
		auto request = parse_in<turner::stun, turner::stun::binding.request>(buffer, *request_message);
		const auto message = std::move(request).to_error({turner::protocol_errc::unauthorized, "abc"}).finish();
		CHECK(hex(message)
			==
			"0111000c" "2112a442" "0102030405060708090a0b0c"
			"00090007" "00000401" "61626300"
		);
	}

	SECTION("to_success: stun binding in turn")
	{
		auto buffer = make_buffer();
		auto request_buffer = make_buffer();
		const auto request_message = writer<turner::turn::binding.request>{request_buffer, stun_id}.finish();
		REQUIRE(request_message);
		auto request = parse_in<turner::turn, turner::turn::binding.request>(buffer, *request_message);
		const auto message = std::move(request).to_success().finish();
		CHECK(hex(message) == "01010000" "2112a442" "0102030405060708090a0b0c");
	}

	SECTION("to_success: turn allocate")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::turn, turner::turn::allocate>(
			buffer,
			turner_test::turn_message::allocate_request
		);
		const auto message = std::move(request).to_success()
			.write(turner::turn::xor_relayed_address, peer_address)
			.write(turner::turn::lifetime, 600s)
			.write(turner::turn::xor_mapped_address, peer_address)
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::allocate_success));
	}

	SECTION("to_error: turn allocate")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::turn, turner::turn::allocate>(
			buffer,
			turner_test::turn_message::allocate_request
		);
		const auto message = std::move(request).to_error(turner::protocol_errc::unauthorized)
			.write(turner::turn::realm, "example.org")
			.write(turner::turn::nonce, "nonce")
			.finish();
		CHECK(hex(message) == hex(turner_test::turn_message::allocate_error));
	}

	SECTION("to_success: msturn allocate")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::msturn, turner::msturn::allocate>(
			buffer,
			turner_test::msturn_message::allocate_request
		);
		const auto message = std::move(request).to_success()
			.write(turner::msturn::mapped_address, server_address)
			.write(turner::msturn::lifetime, 600s)
			.write(turner::msturn::username, "alice")
			.finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::allocate_success));
	}

	SECTION("to_success: msturn set active destination")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::msturn, turner::msturn::set_active_destination>(
			buffer,
			turner_test::msturn_message::set_active_destination_request
		);
		const auto message = std::move(request).to_success().finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::set_active_destination_success));
	}

	SECTION("to_success: msturn set active destination with xor address")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::msturn, turner::msturn::set_active_destination>(
			buffer,
			turner_test::msturn_message::set_active_destination_request
		);
		const auto message = std::move(request).to_success()
			.write(turner::msturn::xor_mapped_address, server_address)
			.finish();
		CHECK(hex(message)
			==
			"01060014" "2112a442" "0102030405060708090a0b0c"
			"000f0004" "72c64bc6"
			"80200008" "00012c84" "e112a643"
		);
	}

	SECTION("to_error: msturn set active destination")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::msturn, turner::msturn::set_active_destination>(
			buffer,
			turner_test::msturn_message::set_active_destination_request
		);
		const auto message = std::move(request).to_error(turner::protocol_errc::unauthorized).finish();
		CHECK(hex(message) == hex(turner_test::msturn_message::set_active_destination_error));
	}

	SECTION("to_error: msturn reason padded with spaces")
	{
		auto buffer = make_buffer();
		auto request = parse_in<turner::msturn, turner::msturn::set_active_destination>(
			buffer,
			turner_test::msturn_message::set_active_destination_request
		);
		const auto message = std::move(request).to_error({turner::protocol_errc::unauthorized, "abc"}).finish();
		CHECK(hex(message)
			==
			"01160014" "2112a442" "0102030405060708090a0b0c"
			"000f0004" "72c64bc6"
			"00090008" "00000401" "61626320"
		);
	}
}

} // namespace
