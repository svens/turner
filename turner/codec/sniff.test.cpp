#include <turner/codec/test.hpp>
#include <turner/codec/sniff.hpp>
#include <turner/codec/protocol.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <concepts>
#include <cstddef>
#include <span>
#include <vector>

namespace
{

using turner::sniff;
using turner::session_protocol;
using turner::transport;
using turner_test::buffer_of;

static_assert(std::same_as<decltype(sniff({}, transport::stream)), session_protocol>);
static_assert(noexcept(sniff({}, transport::stream)));

constexpr auto framed_message_header = turner_test::bytes("02000000");

std::vector<std::byte> framed (std::span<const std::byte> message)
{
	auto stream = buffer_of(framed_message_header);
	stream[2] = static_cast<std::byte>(message.size() >> 8);
	stream[3] = static_cast<std::byte>(message.size());
	stream.insert(stream.end(), message.begin(), message.end());
	return stream;
}

TEST_CASE("sniff")
{
	const auto &stun_request = turner_test::rfc5769::request;
	const auto &stun_response = turner_test::rfc5769::ipv4_response;
	const auto &turn_request = turner_test::turn_message::allocate_request;
	const auto &msturn_request = turner_test::msturn_message::allocate_request;

	SECTION("datagram")
	{
		const auto datagram = [](std::span<const std::byte> bytes)
		{
			return sniff(bytes, transport::datagram);
		};

		SECTION("protocols")
		{
			CHECK(datagram(stun_request) == session_protocol::stun);
			CHECK(datagram(stun_response) == session_protocol::stun);
			CHECK(datagram(turner_test::stun_message::binding_indication) == session_protocol::stun);
			CHECK(datagram(turn_request) == session_protocol::stun);
			CHECK(datagram(msturn_request) == session_protocol::msturn);
			CHECK(datagram(turner_test::msturn_message::data_indication) == session_protocol::msturn);
		}

		SECTION("other")
		{
			static constexpr auto channel_data = turner_test::bytes("40010000");
			static constexpr auto mturn_data = turner_test::bytes("ff100008" "1112131415161718");
			static constexpr auto tls = turner_test::bytes("16030100" "2d010000" "290301");
			CHECK(datagram({}) == session_protocol::other);
			CHECK(datagram(channel_data) == session_protocol::other);
			CHECK(datagram(mturn_data) == session_protocol::other);
			CHECK(datagram(tls) == session_protocol::other);
			CHECK(datagram(std::span{stun_request}.first(19)) == session_protocol::other);
		}

		SECTION("stun cookie changed")
		{
			const auto offset = GENERATE(range(size_t{4}, size_t{8}));
			CAPTURE(offset);
			auto bytes = buffer_of(stun_request);
			bytes[offset] ^= std::byte{0x01};
			CHECK(datagram(bytes) == session_protocol::other);
		}

		SECTION("stun first byte range")
		{
			auto bytes = buffer_of(stun_request);
			bytes[0] = std::byte{3};
			CHECK(datagram(bytes) == session_protocol::stun);
			bytes[0] = std::byte{4};
			CHECK(datagram(bytes) == session_protocol::other);
		}

		SECTION("msturn cookie changed leaves stun cookie value")
		{
			auto bytes = buffer_of(msturn_request);
			bytes[20] ^= std::byte{0x01};
			CHECK(datagram(bytes) == session_protocol::stun);
		}
	}

	SECTION("stream")
	{
		const auto stream = [](std::span<const std::byte> bytes)
		{
			return sniff(bytes, transport::stream);
		};
		const auto msturn_stream = framed(msturn_request);

		SECTION("stun prefixes")
		{
			const std::span<const std::byte> bytes = GENERATE_REF(
				std::span<const std::byte>{stun_request},
				std::span<const std::byte>{stun_response},
				std::span<const std::byte>{turn_request}
			);
			const auto size = GENERATE(range(size_t{0}, size_t{21}));
			CAPTURE(size);
			CHECK(stream(bytes.first(size))
				==
				(size < 8 ? session_protocol::need_more : session_protocol::stun)
			);
		}

		SECTION("msturn prefixes")
		{
			const auto size = GENERATE(range(size_t{0}, size_t{36}));
			CAPTURE(size);
			CHECK(stream(std::span{msturn_stream}.first(size))
				==
				(size < 32 ? session_protocol::need_more : session_protocol::msturn)
			);
		}

		SECTION("stun cookie changed")
		{
			const auto offset = GENERATE(range(size_t{4}, size_t{8}));
			CAPTURE(offset);
			auto bytes = buffer_of(stun_request);
			bytes[offset] ^= std::byte{0x01};
			CHECK(stream(std::span{bytes}.first(offset + 1)) == session_protocol::other);
			CHECK(stream(bytes) == session_protocol::other);
		}

		SECTION("msturn cookie changed")
		{
			const auto offset = GENERATE(range(size_t{24}, size_t{32}));
			CAPTURE(offset);
			auto bytes = msturn_stream;
			bytes[offset] ^= std::byte{0x01};
			CHECK(stream(std::span{bytes}.first(offset + 1)) == session_protocol::other);
			CHECK(stream(bytes) == session_protocol::other);
		}

		SECTION("msturn frame length")
		{
			auto bytes = msturn_stream;
			bytes[2] = std::byte{0};
			bytes[3] = std::byte{27};
			CHECK(stream(std::span{bytes}.first(4)) == session_protocol::other);
			bytes[3] = std::byte{28};
			CHECK(stream(std::span{bytes}.first(4)) == session_protocol::need_more);
			CHECK(stream(bytes) == session_protocol::msturn);
		}

		SECTION("msturn reserved byte is ignored")
		{
			auto bytes = msturn_stream;
			bytes[1] = std::byte{0xff};
			CHECK(stream(bytes) == session_protocol::msturn);
		}

		SECTION("msturn message type bits")
		{
			auto bytes = msturn_stream;
			bytes[4] = std::byte{0x40};
			CHECK(stream(std::span{bytes}.first(5)) == session_protocol::other);
			bytes[4] = std::byte{0x80};
			CHECK(stream(std::span{bytes}.first(5)) == session_protocol::other);
			bytes[4] = std::byte{0x3f};
			CHECK(stream(bytes) == session_protocol::msturn);
		}

		SECTION("other first bytes")
		{
			const auto first = GENERATE(
				std::byte{0x03},
				std::byte{0x04},
				std::byte{0x15},
				std::byte{0x16},
				std::byte{0x17},
				std::byte{0xff}
			);
			CHECK(stream(std::span{&first, 1}) == session_protocol::other);
		}

		SECTION("pseudo-TLS is not session_protocol")
		{
			static constexpr auto tls = turner_test::bytes("16030100" "2d010000");
			CHECK(stream(tls) == session_protocol::other);
		}
	}
}

} // namespace
