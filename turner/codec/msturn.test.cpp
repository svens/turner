#include <turner/codec/test.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <turner/codec/value.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <pal/net/ip/port_type.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <algorithm>
#include <array>
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
using turner::turn;
using turner_test::as_bytes;
using turner_test::buffer_of;
using turner_test::failed;
using turner_test::filler;
using turner_test::hex;
using turner_test::make_buffer;
using turner_test::msturn_id;
using turner_test::parse_as;
using turner_test::peer_address;
using turner_test::server_address;

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

static_assert(sizeof(msturn::transaction_id) == 16);
static_assert(msturn::transaction_id_offset == 4);
static_assert(msturn::header_size == 20);
static_assert(msturn::cookie_offset == 20);
static_assert(
	msturn::cookie == std::array
	{
		std::byte{0x00}, std::byte{0x0f}, std::byte{0x00}, std::byte{0x04},
		std::byte{0x72}, std::byte{0xc6}, std::byte{0x4b}, std::byte{0xc6},
	}
);
static_assert(msturn::header_size == msturn::transaction_id_offset + sizeof(msturn::transaction_id));
static_assert(msturn::attribute_alignment == 1);

static_assert(msturn::allocate.type == 0x0003);
static_assert(msturn::allocate.success.type == 0x0103);
static_assert(msturn::allocate.error.type == 0x0113);
static_assert(msturn::send_request.type == 0x0004);
static_assert(msturn::data_indication.type == 0x0115);
static_assert(msturn::data_indication.indication.type == 0x0115);
static_assert(turner::message_of<decltype(msturn::data_indication)>::type == 0x0115);
static_assert(turner::indication<turn, 0x005>.type == 0x0015);
static_assert(msturn::set_active_destination.type == 0x0006);
static_assert(msturn::set_active_destination.success.type == 0x0106);
static_assert(msturn::set_active_destination.error.type == 0x0116);

static_assert(!has_success<msturn::send_request> && !has_error<msturn::send_request>);
static_assert(!has_success<msturn::data_indication> && !has_error<msturn::data_indication>);

static_assert(turner::message<decltype(msturn::send_request)>);
static_assert(turner::message<decltype(msturn::data_indication)>);

static_assert(turner::usable_with<decltype(msturn::allocate), msturn>);
static_assert(!turner::usable_with<decltype(turn::allocate), msturn>);
static_assert(!turner::usable_with<decltype(stun::binding), msturn>);

static_assert(msturn::frame_header_size == 4);
static_assert(msturn::mturn_header_size == 12);
static_assert(msturn::max_headroom == 60);
static_assert(msturn::max_tailroom == 0);
static_assert(msturn::max_payload == 65479);
static_assert(
	std::same_as<msturn::mturn_session_id, decltype(msturn::multiplexed_session_id)::value_type::native_type>
);
static_assert(!std::is_default_constructible_v<msturn::frame>);
static_assert(!std::is_default_constructible_v<msturn::mturn_data>);
static_assert(noexcept(msturn::peek({})));
static_assert(noexcept(msturn::parse_frame({}, 0)));
static_assert(noexcept(msturn::parse_mturn_data({}, 0)));
static_assert(noexcept(msturn::make_frame({}, {}, msturn::frame_kind::data)));
static_assert(noexcept(msturn::make_data_indication({}, {}, peer_address, msturn_id)));

constexpr turner::transport_address ipv6_peer_address
{
	pal::net::ip::address_v6{
		pal::net::ip::address_v6::bytes_type{0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
	},
	pal::net::ip::port_type{32853},
};

constexpr std::string_view id_hex = "2112a442" "0102030405060708090a0b0c";
constexpr std::string_view cookie_hex = "000f0004" "72c64bc6";
constexpr std::string_view ipv4_peer_hex = "00120008" "00018055" "c0000201";
constexpr std::string_view ipv6_peer_hex =
	"00120014" "00028055" "20010db8" "00000000" "00000000" "00000001";

constexpr auto session_id = turner_test::bytes("1112131415161718");

static_assert(noexcept(msturn::make_mturn_data({}, {}, session_id)));

constexpr size_t payload_offset = 72;

struct payload_case
{
	std::string_view payload;
	std::string_view payload_hex;
	std::string_view ipv4_length_hex;
	std::string_view ipv6_length_hex;
	std::string_view data_hex;
	std::string_view frame_length_hex;
	std::string_view mturn_length_hex;
};

constexpr payload_case payload_cases[] =
{
	{"", "", "0018", "0024", "00130000", "0000", "0008"},
	{"a", "61", "0019", "0025", "00130001" "61", "0001", "0009"},
	{"ab", "6162", "001a", "0026", "00130002" "6162", "0002", "000a"},
	{"abc", "616263", "001b", "0027", "00130003" "616263", "0003", "000b"},
	{"abcd", "61626364", "001c", "0028", "00130004" "61626364", "0004", "000c"},
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

TEST_CASE("msturn")
{
	SECTION("peek")
	{
		static constexpr auto minimal_message = turner_test::bytes(
			"00060008" "2112a442" "01020304" "05060708" "090a0b0c"
			"000f0004" "72c64bc6"
		);
		static constexpr auto mturn_packet = turner_test::bytes("ff10000b" "1112131415161718" "616263");

		SECTION("message")
		{
			CHECK(msturn::peek(minimal_message) == content::message);
			CHECK(msturn::peek(turner_test::msturn_message::allocate_request) == content::message);
			CHECK(msturn::peek(turner_test::msturn_message::data_indication) == content::message);
		}

		SECTION("too short for message")
		{
			CHECK(msturn::peek(std::span{minimal_message}.first(27)) == content::other);
		}

		SECTION("wrong cookie")
		{
			const auto offset = GENERATE(range(size_t{20}, size_t{28}));
			CAPTURE(offset);
			auto bytes = buffer_of(minimal_message);
			bytes[offset] ^= std::byte{0x01};
			CHECK(msturn::peek(bytes) == content::other);
		}

		SECTION("message type high bits")
		{
			const auto first = GENERATE(std::byte{0x40}, std::byte{0x80}, std::byte{0xc0});
			auto bytes = buffer_of(minimal_message);
			bytes[0] = first;
			CHECK(msturn::peek(bytes) == content::other);
		}

		SECTION("raw data")
		{
			static constexpr auto dtls = turner_test::bytes(
				"16fefd00" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000"
			);
			CHECK(msturn::peek({}) == content::other);
			CHECK(msturn::peek(turner_test::rfc5769::request) == content::other);
			CHECK(msturn::peek(dtls) == content::other);
		}

		SECTION("mturn packet")
		{
			CHECK(msturn::peek(mturn_packet) == content::data);
			CHECK(msturn::peek(turner_test::bytes("ff100008" "1112131415161718")) == content::data);
		}

		SECTION("not mturn packet")
		{
			CHECK(msturn::peek(turner_test::bytes("ff")) == content::other);
			CHECK(msturn::peek(std::span{mturn_packet}.first(11)) == content::other);
			CHECK(msturn::peek(std::span{mturn_packet}.first(14)) == content::other);
			static constexpr auto wrong_channel = turner_test::bytes(
				"ff11000b" "1112131415161718" "616263"
			);
			static constexpr auto wrong_length = turner_test::bytes(
				"ff10000a" "1112131415161718" "616263"
			);
			CHECK(msturn::peek(wrong_channel) == content::other);
			CHECK(msturn::peek(wrong_length) == content::other);
		}
	}

	SECTION("parse_frame")
	{
		SECTION("control")
		{
			static constexpr auto bytes = turner_test::bytes("02000003" "616263");
			const auto frame = msturn::parse_frame(bytes, bytes.size());
			REQUIRE(frame);
			CHECK(frame->kind() == msturn::frame_kind::control);
			CHECK(frame->size() == bytes.size());
			CHECK(frame->payload().data() == bytes.data() + 4);
			CHECK(frame->payload().size() == 3);
		}

		SECTION("data")
		{
			static constexpr auto bytes = turner_test::bytes("03000001" "61");
			const auto frame = msturn::parse_frame(bytes, bytes.size());
			REQUIRE(frame);
			CHECK(frame->kind() == msturn::frame_kind::data);
			CHECK(frame->payload().data() == bytes.data() + 4);
			CHECK(frame->payload().size() == 1);
		}

		SECTION("empty payload")
		{
			static constexpr auto bytes = turner_test::bytes("03000000");
			const auto frame = msturn::parse_frame(bytes, bytes.size());
			REQUIRE(frame);
			CHECK(frame->size() == 4);
			CHECK(frame->payload().empty());
		}

		SECTION("reserved byte is ignored")
		{
			static constexpr auto bytes = turner_test::bytes("02ff0000");
			CHECK(msturn::parse_frame(bytes, bytes.size()).has_value());
		}

		SECTION("following bytes")
		{
			static constexpr auto bytes = turner_test::bytes("03000001" "61" "ffff");
			const auto frame = msturn::parse_frame(bytes, bytes.size());
			REQUIRE(frame);
			CHECK(frame->size() == 5);
		}

		SECTION("unknown type")
		{
			const auto type = GENERATE(std::byte{0}, std::byte{1}, std::byte{4}, std::byte{0xff});
			auto bytes = buffer_of(turner_test::bytes("02000000"));
			bytes[0] = type;
			CHECK(msturn::parse_frame(bytes, bytes.size())
				==
				failed(turner::errc::unexpected_message_type)
			);
		}

		SECTION("unknown type is reported before length")
		{
			static constexpr auto bytes = turner_test::bytes("0400ffff");
			CHECK(msturn::parse_frame(bytes, bytes.size())
				==
				failed(turner::errc::unexpected_message_type)
			);
		}

		SECTION("truncated header")
		{
			static constexpr auto bytes = turner_test::bytes("02000000");
			const auto received = GENERATE(range(size_t{0}, size_t{4}));
			CAPTURE(received);
			CHECK(msturn::parse_frame(bytes, received) == failed(turner::errc::incomplete));
		}

		SECTION("truncated payload")
		{
			static constexpr auto bytes = turner_test::bytes("02000003" "616263");
			CHECK(msturn::parse_frame(bytes, bytes.size() - 1) == failed(turner::errc::incomplete));
		}

		SECTION("stream of frames")
		{
			static constexpr auto stream = turner_test::bytes(
				"02000003" "616263"
				"03000000"
				"03000002" "6465"
			);
			const auto received = GENERATE(range(size_t{0}, stream.size() + 1));
			CAPTURE(received);

			size_t consumed = 0;
			size_t frames = 0;
			while (true)
			{
				const auto frame = msturn::parse_frame(
					std::span{stream}.subspan(consumed),
					received - consumed
				);
				if (!frame)
				{
					CHECK(frame.error() == turner::errc::incomplete);
					break;
				}
				consumed += frame->size();
				++frames;
			}
			CHECK(frames == size_t{received >= 7} + size_t{received >= 11} + size_t{received >= 17});
		}
	}

	SECTION("make_frame")
	{
		const auto &c = GENERATE(from_range(payload_cases));
		const auto kind = GENERATE(msturn::frame_kind::control, msturn::frame_kind::data);
		CAPTURE(c.payload);

		auto buffer = make_buffer(128);
		const auto payload = place_payload(buffer, c.payload);
		const auto frame = msturn::make_frame(buffer, payload, kind);

		std::string expected{kind == msturn::frame_kind::control ? "0200" : "0300"};
		expected.append(c.frame_length_hex).append(c.payload_hex);
		CHECK(hex(frame) == expected);
		CHECK(frame.data() == buffer.data() + payload_offset - 4);
		check_only_frame_written(buffer, frame);

		const auto parsed = msturn::parse_frame(frame, frame.size());
		REQUIRE(parsed);
		CHECK(parsed->kind() == kind);
		CHECK(hex(parsed->payload()) == c.payload_hex);
	}

	SECTION("parse_mturn_data")
	{
		SECTION("valid")
		{
			static constexpr auto bytes = turner_test::bytes("ff10000b" "1112131415161718" "616263");
			const auto packet = msturn::parse_mturn_data(bytes, bytes.size());
			REQUIRE(packet);
			CHECK(hex(packet->session_id()) == "1112131415161718");
			CHECK(packet->session_id().data() == bytes.data() + 4);
			CHECK(hex(packet->payload()) == "616263");
			CHECK(packet->payload().data() == bytes.data() + 12);
			CHECK(packet->size() == bytes.size());
		}

		SECTION("empty payload")
		{
			static constexpr auto bytes = turner_test::bytes("ff100008" "1112131415161718");
			const auto packet = msturn::parse_mturn_data(bytes, bytes.size());
			REQUIRE(packet);
			CHECK(packet->payload().empty());
			CHECK(packet->size() == 12);
		}

		SECTION("following bytes")
		{
			static constexpr auto bytes = turner_test::bytes("ff100009" "1112131415161718" "61" "ffff");
			const auto packet = msturn::parse_mturn_data(bytes, bytes.size());
			REQUIRE(packet);
			CHECK(packet->size() == 13);
		}

		SECTION("length below session id")
		{
			static constexpr auto bytes = turner_test::bytes("ff100007" "1112131415161718");
			CHECK(msturn::parse_mturn_data(bytes, bytes.size())
				==
				failed(turner::errc::unexpected_message_length)
			);
		}

		SECTION("wrong channel")
		{
			static constexpr auto bytes = turner_test::bytes("ff110008" "1112131415161718");
			CHECK(msturn::parse_mturn_data(bytes, bytes.size())
				==
				failed(turner::errc::unexpected_message_type)
			);
		}

		SECTION("wrong channel is reported before length")
		{
			static constexpr auto bytes = turner_test::bytes("ff11000b" "1112131415161718");
			CHECK(msturn::parse_mturn_data(bytes, bytes.size())
				==
				failed(turner::errc::unexpected_message_type)
			);
		}

		SECTION("shorter than header")
		{
			static constexpr auto bytes = turner_test::bytes("ff100007" "11121314151617");
			CHECK(msturn::parse_mturn_data(bytes, bytes.size()) == failed(turner::errc::incomplete));
		}

		SECTION("truncated")
		{
			static constexpr auto bytes = turner_test::bytes("ff10000b" "1112131415161718" "616263");
			const auto received = GENERATE(size_t{0}, size_t{11}, size_t{12}, size_t{14});
			CAPTURE(received);
			CHECK(msturn::parse_mturn_data(bytes, received) == failed(turner::errc::incomplete));
		}
	}

	SECTION("make_mturn_data")
	{
		SECTION("payload sizes")
		{
			const auto &c = GENERATE(from_range(payload_cases));
			CAPTURE(c.payload);

			auto buffer = make_buffer(128);
			const auto payload = place_payload(buffer, c.payload);
			const auto packet = msturn::make_mturn_data(buffer, payload, session_id);

			std::string expected{"ff10"};
			expected.append(c.mturn_length_hex).append("1112131415161718").append(c.payload_hex);
			CHECK(hex(packet) == expected);
			CHECK(packet.data() == buffer.data() + payload_offset - 12);
			check_only_frame_written(buffer, packet);

			const auto parsed = msturn::parse_mturn_data(packet, packet.size());
			REQUIRE(parsed);
			CHECK(hex(parsed->session_id()) == "1112131415161718");
			CHECK(hex(parsed->payload()) == c.payload_hex);
		}

		SECTION("session id in bytes being written")
		{
			const auto id_distance = GENERATE(size_t{12}, size_t{14});
			CAPTURE(id_distance);

			auto buffer = make_buffer(128);
			const auto payload = place_payload(buffer, "abc");
			const auto id_bytes = std::span{buffer}.subspan(payload_offset - id_distance).first<8>();
			std::ranges::copy(session_id, id_bytes.begin());

			const msturn::mturn_session_id id{id_bytes};
			CHECK(hex(msturn::make_mturn_data(buffer, payload, id))
				==
				"ff10000b" "1112131415161718" "616263"
			);
		}
	}

	SECTION("make_data_indication")
	{
		const auto &c = GENERATE(from_range(payload_cases));
		const bool ipv6 = GENERATE(false, true);
		CAPTURE(c.payload, ipv6);

		const auto &peer = ipv6 ? ipv6_peer_address : peer_address;
		const size_t prefix = ipv6 ? 56 : 44;

		auto buffer = make_buffer(128);
		const auto payload = place_payload(buffer, c.payload);
		const auto message = msturn::make_data_indication(buffer, payload, peer, msturn_id);

		std::string expected{"0115"};
		expected.append(ipv6 ? c.ipv6_length_hex : c.ipv4_length_hex)
			.append(id_hex)
			.append(cookie_hex)
			.append(ipv6 ? ipv6_peer_hex : ipv4_peer_hex)
			.append(c.data_hex);
		CHECK(hex(message) == expected);
		CHECK(message.data() == buffer.data() + payload_offset - prefix);
		check_only_frame_written(buffer, message);

		std::vector<std::byte> copy;
		const auto indication = parse_as<msturn, msturn::data_indication>(message, copy);
		CHECK(indication.transaction_id() == msturn_id);
		CHECK(indication.read(msturn::remote_address) == peer);
		const auto data = indication.read(msturn::data);
		REQUIRE(data);
		CHECK(hex(*data) == c.payload_hex);
	}

	SECTION("make_data_indication: existing vector")
	{
		auto buffer = make_buffer(128);
		const auto payload = place_payload(buffer, "abc");
		CHECK(hex(msturn::make_data_indication(buffer, payload, server_address, msturn_id))
			==
			hex(turner_test::msturn_message::data_indication)
		);
	}

	SECTION("max_payload")
	{
		std::vector<std::byte> buffer(msturn::max_headroom + msturn::max_payload, filler);
		const auto payload = std::span<const std::byte>{buffer}.subspan(msturn::max_headroom);

		const auto message = msturn::make_data_indication(buffer, payload, ipv6_peer_address, msturn_id);
		CHECK(hex(message.first(4)) == "0115ffeb");

		const auto frame = msturn::make_frame(buffer, message, msturn::frame_kind::control);
		CHECK(frame.size() == buffer.size());
		CHECK(hex(frame.first(4)) == "0200ffff");

		const auto parsed = msturn::parse_frame(frame, frame.size());
		REQUIRE(parsed);
		std::vector<std::byte> copy;
		const auto indication = parse_as<msturn, msturn::data_indication>(parsed->payload(), copy);
		const auto data = indication.read(msturn::data);
		REQUIRE(data);
		CHECK(data->size() == msturn::max_payload);
	}
}

} // namespace
