#include <turner/stun>
#include <turner/test>
#include <turner/error>


namespace {

using namespace turner_test;


TEST_CASE("stun")
{
	SECTION("method registry")
	{
		static_assert(turner::stun::binding.method == 0x0001);
		static_assert(turner::stun::binding.type == 0x0001);
		static_assert(turner::stun::binding.success.method == 0x0001);
		static_assert(turner::stun::binding.success.type == 0x0101);
	}

	SECTION("attribute registry")
	{
		static_assert(turner::stun::mapped_address == 0x0001);
		static_assert(turner::stun::username == 0x0006);
		static_assert(turner::stun::message_integrity == 0x0008);
		static_assert(turner::stun::error_code == 0x0009);
		static_assert(turner::stun::unknown_attributes == 0x000a);
		static_assert(turner::stun::realm == 0x0014);
		static_assert(turner::stun::nonce == 0x0015);
		static_assert(turner::stun::xor_mapped_address == 0x0020);

		static_assert(turner::stun::alternate_domain == 0x8003);
		static_assert(turner::stun::software == 0x8022);
		static_assert(turner::stun::alternate_server == 0x8023);
		static_assert(turner::stun::fingerprint == 0x8028);
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
