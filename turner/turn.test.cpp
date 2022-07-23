#include <turner/turn>
#include <turner/msturn>
#include <turner/test>


namespace {

using namespace turner_test;


TEST_CASE("turn")
{
	static_assert(std::is_convertible_v<turner::turn, turner::stun>);
	static_assert(!std::is_convertible_v<turner::msturn, turner::stun>);

	SECTION("method registry")
	{
		CHECK(turner::turn::allocate == 0x0003_u16);
		CHECK(turner::turn::allocate_success == 0x0103_u16);
		CHECK(turner::turn::allocate_error == 0x0113_u16);

		CHECK(turner::turn::refresh == 0x0004_u16);
		CHECK(turner::turn::refresh_success == 0x0104_u16);
		CHECK(turner::turn::refresh_error == 0x0114_u16);

		CHECK(turner::turn::send_indication == 0x0006_u16);

		CHECK(turner::turn::data_indication == 0x0007_u16);

		CHECK(turner::turn::create_permission == 0x0008_u16);
		CHECK(turner::turn::create_permission_success == 0x0108_u16);
		CHECK(turner::turn::create_permission_error == 0x0118_u16);

		CHECK(turner::turn::channel_bind == 0x0009_u16);
		CHECK(turner::turn::channel_bind_success == 0x0109_u16);
		CHECK(turner::turn::channel_bind_error == 0x0119_u16);
	}

	SECTION("attribute registry")
	{
		CHECK(turner::turn::channel_number == 0x000c_u16);
		CHECK(turner::turn::lifetime == 0x000d_u16);
		CHECK(turner::turn::xor_peer_address == 0x0012_u16);
		CHECK(turner::turn::data == 0x0013_u16);
		CHECK(turner::turn::xor_relayed_address == 0x0016_u16);
		CHECK(turner::turn::requested_address_family == 0x0017_u16);
		CHECK(turner::turn::even_port == 0x0018_u16);
		CHECK(turner::turn::requested_transport == 0x0019_u16);
		CHECK(turner::turn::dont_fragment == 0x001a_u16);
		CHECK(turner::turn::reservation_token == 0x0022_u16);
		CHECK(turner::turn::additional_address_family == 0x8000_u16);
		CHECK(turner::turn::address_error_code == 0x8001_u16);
		CHECK(turner::turn::icmp == 0x8004_u16);
	}

	SECTION("STUN interoperability")
	{
		constexpr uint8_t data[] =
		{
			0x00, 0x03, 0x00, 0x08, // TURN Allocate
			0x21, 0x12, 0xa4, 0x42, // Magic Cookie
			0x00, 0x01, 0x02, 0x03, // Transaction ID
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,
			0x80, 0x28, 0x00, 0x04, // Fingerprint
			0x2f, 0xbb, 0xf0, 0x0d,
		};

		auto span = std::as_bytes(std::span{data});

		auto turn_reader = turner::turn::read_message(span);
		REQUIRE(turn_reader);
		CHECK(turn_reader->message_type() == turner::turn::allocate);
		CHECK(turn_reader->message_type() != turner::stun::binding);

		auto stun_reader = turner::stun::read_message(span);
		REQUIRE(stun_reader);
		CHECK(stun_reader->message_type() == turner::turn::allocate);
		CHECK(stun_reader->message_type() != turner::stun::binding);
	}
}


} // namespace
