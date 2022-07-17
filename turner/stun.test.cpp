#include <turner/stun>
#include <turner/test>
#include <turner/error>
#include <catch2/generators/catch_generators.hpp>


namespace {

using namespace turner_test;

// https://tools.ietf.org/html/rfc5769 test vector
constexpr uint8_t message_data[] =
{
	0x00, 0x01, 0x00, 0x58, // Binding

	0x21, 0x12, 0xa4, 0x42, // Magic cookie

	0xb7, 0xe7, 0xa7, 0x01, // Transaction ID
	0xbc, 0x34, 0xd6, 0x86,
	0xfa, 0x87, 0xdf, 0xae,

	0x80, 0x22, 0x00, 0x10, // SOFTWARE
	0x53, 0x54, 0x55, 0x4e, // "STUN test client" (no quotes)
	0x20, 0x74, 0x65, 0x73,
	0x74, 0x20, 0x63, 0x6c,
	0x69, 0x65, 0x6e, 0x74,

	0x00, 0x24, 0x00, 0x04, // PRIORITY
	0x6e, 0x00, 0x01, 0xff,

	0x80, 0x29, 0x00, 0x08, // ICE-CONTROLLED
	0x93, 0x2f, 0xf9, 0xb1,
	0x51, 0x26, 0x3b, 0x36,

	0x00, 0x06, 0x00, 0x09, // USERNAME
	0x65, 0x76, 0x74, 0x6a, // "evtj:h6vY" (no quotes)
	0x3a, 0x68, 0x36, 0x76,
	0x59, 0x20, 0x20, 0x20,

	0x00, 0x08, 0x00, 0x14, // MESSAGE-INTEGRITY
	0x9a, 0xea, 0xa7, 0x0c,
	0xbf, 0xd8, 0xcb, 0x56,
	0x78, 0x1e, 0xf2, 0xb5,
	0xb2, 0xd3, 0xf2, 0x49,
	0xc1, 0xb5, 0x71, 0xa2,

	0x80, 0x28, 0x00, 0x04, // FINGERPRINT
	0xe5, 0x7a, 0x3b, 0xcf,
};

const auto message_span = std::as_bytes(std::span{message_data});


TEST_CASE("stun")
{
	SECTION("method registry")
	{
		CHECK(turner::stun::binding == 0x001_u16);
		CHECK(turner::stun::binding_success == 0x101_u16);
	}

	SECTION("attribute registry")
	{
		CHECK(turner::stun::mapped_address == 0x0001_u16);
		CHECK(turner::stun::username == 0x0006_u16);
		CHECK(turner::stun::message_integrity == 0x0008_u16);
		CHECK(turner::stun::error_code == 0x0009_u16);
		CHECK(turner::stun::unknown_attributes == 0x000a_u16);
		CHECK(turner::stun::realm == 0x0014_u16);
		CHECK(turner::stun::nonce == 0x0015_u16);
		CHECK(turner::stun::message_integrity_sha256 == 0x001c_u16);
		CHECK(turner::stun::password_algorithm == 0x001d_u16);
		CHECK(turner::stun::userhash == 0x001e_u16);
		CHECK(turner::stun::xor_mapped_address == 0x0020_u16);

		CHECK(turner::stun::password_algorithms == 0x8002_u16);
		CHECK(turner::stun::alternate_domain == 0x8003_u16);
		CHECK(turner::stun::software == 0x8022_u16);
		CHECK(turner::stun::alternate_server == 0x8023_u16);
		CHECK(turner::stun::fingerprint == 0x8028_u16);
	}

	SECTION("read success")
	{
		turner::stun::transaction_id_type expected_transaction_id =
		{
			0xb7, 0xe7, 0xa7, 0x01,
			0xbc, 0x34, 0xd6, 0x86,
			0xfa, 0x87, 0xdf, 0xae,
		};

		auto r = pal_try(turner::stun::read_message(message_span));
		CHECK(r.message_type() == turner::stun::binding);
		CHECK(r.transaction_id() == expected_transaction_id);

		auto s = r.as_bytes();
		CHECK(s.data() == message_span.data());
		CHECK(s.size_bytes() == message_span.size_bytes());
	}

	SECTION("read insufficient span")
	{
		uint8_t data[] = { 0x00, 0x01, };
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::unexpected_message_length);
	}

	SECTION("read unpadded span")
	{
		uint8_t data[21] = { 0x00, 0x01, };
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::unexpected_message_length);
	}

	SECTION("read invalid message magic cookie")
	{
		uint8_t data[20] = { 0x00, 0x01, 0x00, 0x00, 0x20, 0x12, 0xa4, 0x42, };
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::invalid_magic_cookie);
	}

	SECTION("read unexpected message length")
	{
		uint8_t data[20] = { 0x00, 0x01, 0x00, 0x04, 0x21, 0x12, 0xa4, 0x42, };
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::unexpected_message_length);
	}

	SECTION("read unpadded message length")
	{
		uint8_t data[24] = { 0x00, 0x01, 0x00, 0x03, 0x21, 0x12, 0xa4, 0x42, };
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::unexpected_message_length);
	}

	SECTION("read unexpected message type")
	{
		uint8_t data[20] = { 0xf0, 0x01, 0x00, 0x00, 0x21, 0x12, 0xa4, 0x42, };
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::unexpected_message_type);
	}

	SECTION("read unexpected attribute length")
	{
		uint8_t data[] =
		{
			0x00, 0x01, 0x00, 0x04, // Mesage Type, Length
			0x21, 0x12, 0xa4, 0x42, // Magic Cookie
			0x00, 0x01, 0x02, 0x03, // Transaction ID
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,

			0x00, 0x01, 0x00, 0x05, // Attribute Type, Length
		};
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::unexpected_attribute_length);
	}

	SECTION("read fingerprint not last")
	{
		constexpr uint8_t data[] =
		{
			0x00, 0x01, 0x00, 0x0c, // Message Type, Length
			0x21, 0x12, 0xa4, 0x42, // Magic Cookie
			0x00, 0x01, 0x02, 0x03, // Transaction ID
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,

			0x80, 0x28, 0x00, 0x04, // FINGERPRINT, Length
			0x01, 0x02, 0x03, 0x04,

			0x00, 0x01, 0x00, 0x00, // MAPPED-ADDRESS, Length
						// no value but doesn't matter
						// for this test
		};
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::fingerprint_not_last);
	}

	SECTION("read unexpected fingerprint value")
	{
		constexpr uint8_t data[] =
		{
			0x00, 0x01, 0x00, 0x08, // Message Type, Length
			0x21, 0x12, 0xa4, 0x42, // Magic Cookie
			0x00, 0x01, 0x02, 0x03, // Transaction ID
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,

			0x80, 0x28, 0x00, 0x04, // FINGERPRINT, Length
			0x01, 0x02, 0x03, 0x04,
		};
		auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!r);
		CHECK(r.error() == turner::errc::fingerprint_mismatch);
	}
}


} // namespace
