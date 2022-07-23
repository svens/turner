#include <turner/stun>
#include <turner/test>
#include <turner/error>


namespace {

using namespace turner_test;


TEST_CASE("stun")
{
	SECTION("method registry")
	{
		CHECK(turner::stun::binding == 0x0001_u16);
		CHECK(turner::stun::binding_success == 0x0101_u16);
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
		CHECK(turner::stun::xor_mapped_address == 0x0020_u16);

		CHECK(turner::stun::alternate_domain == 0x8003_u16);
		CHECK(turner::stun::software == 0x8022_u16);
		CHECK(turner::stun::alternate_server == 0x8023_u16);
		CHECK(turner::stun::fingerprint == 0x8028_u16);
	}

	SECTION("read_message")
	{
		SECTION("fingerprint not last")
		{
			constexpr uint8_t data[] =
			{
				0x00, 0x01, 0x00, 0x10, // STUN Binding
				0x21, 0x12, 0xa4, 0x42, // Magic Cookie
				0x00, 0x01, 0x02, 0x03, // Transaction ID
				0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0a, 0x0b,

				0x80, 0x28, 0x00, 0x04, // FINGERPRINT
				0x01, 0x02, 0x03, 0x04,

				0x80, 0x22, 0x00, 0x04, // SOFTWARE
				'S',  'T',  'U',  'N',
			};
			auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
			REQUIRE(!r);
			CHECK(r.error() == turner::errc::fingerprint_not_last);
		}

		SECTION("fingerprint mismatch")
		{
			constexpr uint8_t data[] =
			{
				0x00, 0x01, 0x00, 0x08, // STUN Binding
				0x21, 0x12, 0xa4, 0x42, // Magic Cookie
				0x00, 0x01, 0x02, 0x03, // Transaction ID
				0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0a, 0x0b,

				0x80, 0x28, 0x00, 0x04, // FINGERPRINT
				0xff, 0xff, 0xff, 0xff, // invalid value
			};
			auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
			REQUIRE(!r);
			CHECK(r.error() == turner::errc::fingerprint_mismatch);
		}

		SECTION("fingerprint unexpected length")
		{
			constexpr uint8_t data[] =
			{
				0x00, 0x01, 0x00, 0x08, // STUN Binding
				0x21, 0x12, 0xa4, 0x42, // Magic Cookie
				0x00, 0x01, 0x02, 0x03, // Transaction ID
				0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0a, 0x0b,

				0x80, 0x28, 0x00, 0x03, // FINGERPRINT, invalid length
				0x5b, 0x0f, 0xf6, 0xfc,
			};
			auto r = turner::stun::read_message(std::as_bytes(std::span{data}));
			REQUIRE(!r);
			CHECK(r.error() == turner::errc::unexpected_attribute_length);
		}
	}
}


} // namespace
