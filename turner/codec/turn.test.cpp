#include <turner/codec/test.hpp>
#include <turner/codec/turn.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/value.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <pal/net/ip/port_type.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace
{

using turner::content;
using turner::msturn;
using turner::stun;
using turner::transport;
using turner::turn;
using turner_test::as_bytes;
using turner_test::buffer_of;
using turner_test::failed;
using turner_test::filler;
using turner_test::hex;
using turner_test::make_buffer;
using turner_test::parse_as;
using turner_test::peer_address;
using turner_test::stun_id;

template <auto M>
concept has_indication = requires
{
	M.indication;
};

template <auto M>
concept has_success = requires
{
	M.success;
};

template <auto M>
concept has_error = requires
{
	M.error;
};

static_assert(turn::allocate.type == 0x0003);
static_assert(turn::allocate.success.type == 0x0103);
static_assert(turn::allocate.error.type == 0x0113);
static_assert(turn::refresh.type == 0x0004);
static_assert(turn::refresh.success.type == 0x0104);
static_assert(turn::refresh.error.type == 0x0114);
static_assert(turn::send_indication.type == 0x0016);
static_assert(turn::data_indication.type == 0x0017);
static_assert(turn::create_permission.type == 0x0008);
static_assert(turn::create_permission.success.type == 0x0108);
static_assert(turn::create_permission.error.type == 0x0118);
static_assert(turn::channel_bind.type == 0x0009);
static_assert(turn::channel_bind.success.type == 0x0109);
static_assert(turn::channel_bind.error.type == 0x0119);
static_assert(turn::binding.request.type == 0x0001);
static_assert(std::same_as<decltype(turn::binding), decltype(stun::binding)>);
static_assert(turn::header_size == stun::header_size);

static_assert(!has_indication<turn::allocate>);
static_assert(!has_success<turn::send_indication> && !has_error<turn::send_indication>);
static_assert(!has_success<turn::data_indication> && !has_error<turn::data_indication>);

static_assert(turner::message<decltype(turn::allocate)>);
static_assert(turner::message<decltype(turn::send_indication)>);
static_assert(!turner::message<decltype(turn::binding)>);

static_assert(turner::usable_with<decltype(turn::allocate), turn>);
static_assert(turner::usable_with<decltype(turn::binding), turn>);
static_assert(turner::usable_with<decltype(turn::binding.request), turn>);
static_assert(!turner::usable_with<decltype(turn::allocate), stun>);
static_assert(!turner::usable_with<decltype(msturn::allocate), turn>);

static_assert(turn::max_headroom == 48);
static_assert(turn::max_tailroom == 3);
static_assert(turn::max_payload == 65504);
static_assert(!std::is_default_constructible_v<turn::channel_data>);
static_assert(std::is_trivially_copyable_v<turn::channel_data>);
static_assert(noexcept(turn::peek({})));
static_assert(noexcept(turn::parse_channel_data({}, 0, transport::datagram)));
static_assert(noexcept(turn::make_channel_data({}, {}, 0)));
static_assert(noexcept(turn::make_data_indication({}, {}, peer_address, stun_id)));

constexpr turner::transport_address ipv6_peer_address
{
	pal::net::ip::address_v6{
		pal::net::ip::address_v6::bytes_type{0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
	},
	pal::net::ip::port_type{32853},
};

constexpr std::string_view id_hex = "2112a442" "0102030405060708090a0b0c";
constexpr std::string_view ipv4_peer_hex = "00120008" "0001a147" "e112a643";
constexpr std::string_view ipv6_peer_hex =
	"00120014" "0002a147" "0113a9fa" "01020304" "05060708" "090a0b0d";

constexpr size_t payload_offset = 64;

struct payload_case
{
	std::string_view payload;
	std::string_view channel_data_hex;
	std::string_view ipv4_length_hex;
	std::string_view ipv6_length_hex;
	std::string_view data_hex;
};

constexpr payload_case payload_cases[] =
{
	{"", "40010000", "0010", "001c", "00130000"},
	{"a", "40010001" "61000000", "0014", "0020", "00130001" "61000000"},
	{"ab", "40010002" "61620000", "0014", "0020", "00130002" "61620000"},
	{"abc", "40010003" "61626300", "0014", "0020", "00130003" "61626300"},
	{"abcd", "40010004" "61626364", "0014", "0020", "00130004" "61626364"},
};

std::span<const std::byte> place_payload (std::vector<std::byte> &buffer, std::string_view payload)
{
	const auto bytes = as_bytes(payload);
	std::ranges::copy(bytes, buffer.begin() + payload_offset);
	return std::span<const std::byte>{buffer}.subspan(payload_offset, bytes.size());
}

void check_only_frame_written (std::span<const std::byte> buffer, std::span<const std::byte> frame)
{
	const auto before = static_cast<size_t>(frame.data() - buffer.data());
	CHECK(std::ranges::count(buffer.first(before), filler) == static_cast<std::ptrdiff_t>(before));

	const auto after = buffer.subspan(before + frame.size());
	CHECK(std::ranges::count(after, filler) == static_cast<std::ptrdiff_t>(after.size()));
}

TEST_CASE("turn")
{
	SECTION("peek")
	{
		const auto first_byte = [](unsigned value)
		{
			return std::vector<std::byte>{static_cast<std::byte>(value)};
		};

		CHECK(turn::peek({}) == content::need_more);
		CHECK(turn::peek(first_byte(0)) == content::message);
		CHECK(turn::peek(first_byte(3)) == content::message);
		CHECK(turn::peek(first_byte(4)) == content::other);
		CHECK(turn::peek(first_byte(63)) == content::other);
		CHECK(turn::peek(first_byte(64)) == content::data);
		CHECK(turn::peek(first_byte(79)) == content::data);
		CHECK(turn::peek(first_byte(80)) == content::other);
		CHECK(turn::peek(first_byte(255)) == content::other);
		CHECK(turn::peek(turner_test::turn_message::allocate_request) == content::message);
		CHECK(turn::peek(turner_test::bytes("40010000")) == content::data);
	}

	SECTION("parse_channel_data")
	{
		const auto kind = GENERATE(transport::datagram, transport::stream);
		CAPTURE(kind);

		SECTION("padded")
		{
			static constexpr auto bytes = turner_test::bytes("40010005" "68656c6c" "6f000000");
			const auto message = turn::parse_channel_data(bytes, bytes.size(), kind);
			REQUIRE(message);
			CHECK(message->channel() == 0x4001);
			CHECK(message->size() == bytes.size());
			CHECK(message->payload().data() == bytes.data() + 4);
			CHECK(message->payload().size() == 5);
		}

		SECTION("empty payload")
		{
			static constexpr auto bytes = turner_test::bytes("40010000");
			const auto message = turn::parse_channel_data(bytes, bytes.size(), kind);
			REQUIRE(message);
			CHECK(message->size() == 4);
			CHECK(message->payload().empty());
			CHECK(message->payload().data() == bytes.data() + 4);
		}

		SECTION("following bytes")
		{
			static constexpr auto bytes = turner_test::bytes("40010001" "61000000" "ffffffff");
			const auto message = turn::parse_channel_data(bytes, bytes.size(), kind);
			REQUIRE(message);
			CHECK(message->size() == 8);
		}

		SECTION("bytes beyond received are not used")
		{
			static constexpr auto bytes = turner_test::bytes("40010004" "61626364");
			CHECK(turn::parse_channel_data(bytes, 7, kind) == failed(turner::errc::incomplete));
		}

		SECTION("truncated header")
		{
			static constexpr auto bytes = turner_test::bytes("40010000");
			const auto received = GENERATE(range(size_t{0}, size_t{4}));
			CAPTURE(received);
			CHECK(turn::parse_channel_data(bytes, received, kind) == failed(turner::errc::incomplete));
		}

		SECTION("truncated payload")
		{
			static constexpr auto bytes = turner_test::bytes("40010005" "68656c6c");
			CHECK(turn::parse_channel_data(bytes, bytes.size(), kind) == failed(turner::errc::incomplete));
		}

		SECTION("channel number")
		{
			auto bytes = buffer_of(turner_test::bytes("00000000"));

			const auto valid = GENERATE(0x4000u, 0x4fffu);
			bytes[0] = static_cast<std::byte>(valid >> 8);
			bytes[1] = static_cast<std::byte>(valid);
			const auto message = turn::parse_channel_data(bytes, bytes.size(), kind);
			REQUIRE(message);
			CHECK(message->channel() == valid);

			const auto invalid = GENERATE(0x3fffu, 0x5000u, 0x7fffu, 0xffffu);
			bytes[0] = static_cast<std::byte>(invalid >> 8);
			bytes[1] = static_cast<std::byte>(invalid);
			CHECK(turn::parse_channel_data(bytes, bytes.size(), kind)
				==
				failed(turner::errc::unexpected_message_type)
			);
		}
	}

	SECTION("parse_channel_data: padding")
	{
		static constexpr auto bytes = turner_test::bytes("40010005" "68656c6c" "6f000000");
		const auto received = GENERATE(size_t{9}, size_t{10}, size_t{11});
		CAPTURE(received);

		CHECK(turn::parse_channel_data(bytes, received, transport::stream) == failed(turner::errc::incomplete));

		const auto message = turn::parse_channel_data(bytes, received, transport::datagram);
		REQUIRE(message);
		CHECK(message->size() == received);
		CHECK(message->payload().size() == 5);
	}

	SECTION("parse_channel_data: stream of messages")
	{
		const auto &request = turner_test::turn_message::allocate_request;
		static constexpr auto channel_data = turner_test::bytes("40010005" "68656c6c" "6f000000");

		auto stream = buffer_of(request);
		stream.insert(stream.end(), channel_data.begin(), channel_data.end());
		stream.insert(stream.end(), request.begin(), request.end());

		const auto received = GENERATE_COPY(range(size_t{0}, stream.size() + 1));
		CAPTURE(received);

		size_t consumed = 0;
		size_t messages = 0;
		size_t frames = 0;
		while (true)
		{
			const auto rest = std::span{stream}.subspan(consumed);
			const auto available = received - consumed;
			const auto kind = turn::peek(rest.first(available));
			if (kind == content::message)
			{
				const auto message = turn::parse(rest, available);
				if (!message)
				{
					CHECK(message.error() == turner::errc::incomplete);
					break;
				}
				consumed += message->size();
				++messages;
			}
			else if (kind == content::data)
			{
				const auto message = turn::parse_channel_data(rest, available, transport::stream);
				if (!message)
				{
					CHECK(message.error() == turner::errc::incomplete);
					break;
				}
				CHECK(message->payload().size() == 5);
				consumed += message->size();
				++frames;
			}
			else
			{
				CHECK(kind == content::need_more);
				CHECK(available == 0);
				break;
			}
		}

		const auto first_end = request.size();
		const auto second_end = first_end + channel_data.size();
		const auto third_end = second_end + request.size();
		CHECK(messages == size_t{received >= first_end} + size_t{received >= third_end});
		CHECK(frames == size_t{received >= second_end});
	}

	SECTION("make_channel_data")
	{
		const auto &c = GENERATE(from_range(payload_cases));
		CAPTURE(c.payload);

		auto buffer = make_buffer(128);
		const auto payload = place_payload(buffer, c.payload);
		const auto frame = turn::make_channel_data(buffer, payload, 0x4001);

		CHECK(hex(frame) == c.channel_data_hex);
		CHECK(frame.data() == buffer.data() + payload_offset - 4);
		check_only_frame_written(buffer, frame);

		const auto message = turn::parse_channel_data(frame, frame.size(), transport::stream);
		REQUIRE(message);
		CHECK(message->channel() == 0x4001);
		CHECK(message->size() == frame.size());
		CHECK(hex(message->payload()) == hex(as_bytes(c.payload)));
	}

	SECTION("make_data_indication")
	{
		const auto &c = GENERATE(from_range(payload_cases));
		const bool ipv6 = GENERATE(false, true);
		CAPTURE(c.payload, ipv6);

		const auto &peer = ipv6 ? ipv6_peer_address : peer_address;
		const auto length_hex = ipv6 ? c.ipv6_length_hex : c.ipv4_length_hex;
		const auto peer_hex = ipv6 ? ipv6_peer_hex : ipv4_peer_hex;
		const size_t prefix = ipv6 ? 48 : 36;

		auto buffer = make_buffer(128);
		const auto payload = place_payload(buffer, c.payload);
		const auto frame = turn::make_data_indication(buffer, payload, peer, stun_id);

		std::string expected{"0017"};
		expected.append(length_hex).append(id_hex).append(peer_hex).append(c.data_hex);
		CHECK(hex(frame) == expected);
		CHECK(frame.data() == buffer.data() + payload_offset - prefix);
		check_only_frame_written(buffer, frame);

		std::vector<std::byte> copy;
		const auto indication = parse_as<turn, turn::data_indication>(frame, copy);
		CHECK(indication.transaction_id() == stun_id);
		CHECK(indication.read(turn::xor_peer_address) == peer);
		const auto data = indication.read(turn::data);
		REQUIRE(data);
		CHECK(hex(*data) == hex(as_bytes(c.payload)));
	}

	SECTION("make_data_indication: existing vector")
	{
		auto buffer = make_buffer(128);
		const auto payload = place_payload(buffer, "hello");
		CHECK(hex(turn::make_data_indication(buffer, payload, peer_address, stun_id))
			==
			hex(turner_test::turn_message::data_indication)
		);
	}

	SECTION("max_payload")
	{
		std::vector<std::byte> buffer(turn::max_headroom + turn::max_payload + turn::max_tailroom, filler);
		const auto payload = std::span<const std::byte>{buffer}.subspan(turn::max_headroom, turn::max_payload);

		const auto indication_frame = turn::make_data_indication(buffer, payload, ipv6_peer_address, stun_id);
		CHECK(indication_frame.size() == buffer.size() - turn::max_tailroom);
		CHECK(hex(indication_frame.first(4)) == "0017fffc");

		std::vector<std::byte> copy;
		const auto indication = parse_as<turn, turn::data_indication>(indication_frame, copy);
		const auto data = indication.read(turn::data);
		REQUIRE(data);
		CHECK(data->size() == turn::max_payload);

		const auto channel_frame = turn::make_channel_data(buffer, payload, 0x4001);
		CHECK(hex(channel_frame.first(4)) == "4001ffe0");
		const auto message = turn::parse_channel_data(channel_frame, channel_frame.size(), transport::stream);
		REQUIRE(message);
		CHECK(message->payload().size() == turn::max_payload);
	}
}

} // namespace
