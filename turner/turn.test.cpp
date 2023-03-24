#include <turner/turn>
#include <turner/msturn>
#include <turner/test>


namespace {

using namespace turner_test;

template <typename ValueType>
using test_message = turner_test::test_message<turner::turn, ValueType>;


TEST_CASE("turn")
{
	static_assert(std::is_convertible_v<turner::turn, turner::stun>);
	static_assert(!std::is_convertible_v<turner::msturn, turner::stun>);

	SECTION("method registry") //{{{1
	{
		// Allocate
		static_assert(turner::turn::allocate.method == 0x0003);
		static_assert(turner::turn::allocate.type == 0x0003);
		static_assert(turner::turn::allocate.success.type == 0x0103);
		static_assert(turner::turn::allocate.error.type == 0x0113);

		// Refresh
		static_assert(turner::turn::refresh.method == 0x0004);
		static_assert(turner::turn::refresh.type == 0x0004);
		static_assert(turner::turn::refresh.success.type == 0x0104);
		static_assert(turner::turn::refresh.error.type == 0x0114);

		// SendIndication
		static_assert(turner::turn::send_indication.method == 0x0006);
		static_assert(turner::turn::send_indication.type == 0x0016);

		// DataIndication
		static_assert(turner::turn::data_indication.method == 0x0007);
		static_assert(turner::turn::data_indication.type == 0x0017);

		// CreatePermission
		static_assert(turner::turn::create_permission.method == 0x0008);
		static_assert(turner::turn::create_permission.type == 0x0008);
		static_assert(turner::turn::create_permission.success.type == 0x0108);
		static_assert(turner::turn::create_permission.error.type == 0x0118);

		// ChannelBind
		static_assert(turner::turn::channel_bind.method == 0x0009);
		static_assert(turner::turn::channel_bind.type == 0x0009);
		static_assert(turner::turn::channel_bind.success.type == 0x0109);
		static_assert(turner::turn::channel_bind.error.type == 0x0119);
	}

	SECTION("attribute registry") //{{{1
	{
		static_assert(turner::turn::channel_number == 0x000c);
		static_assert(turner::turn::lifetime == 0x000d);
		static_assert(turner::turn::xor_peer_address == 0x0012);
		static_assert(turner::turn::data == 0x0013);
		static_assert(turner::turn::xor_relayed_address == 0x0016);
		static_assert(turner::turn::requested_address_family == 0x0017);
		static_assert(turner::turn::even_port == 0x0018);
		static_assert(turner::turn::requested_transport == 0x0019);
		static_assert(turner::turn::dont_fragment == 0x001a);
		static_assert(turner::turn::reservation_token == 0x0022);
		static_assert(turner::turn::additional_address_family == 0x8000);
		static_assert(turner::turn::address_error_code == 0x8001);
	}

	SECTION("STUN interoperability") //{{{1
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

		// TURN reader can check both TURN & STUN messages
		auto turn_reader = turner::turn::read_message(span);
		REQUIRE(turn_reader);
		CHECK(turn_reader->expect(turner::turn::allocate));
		CHECK_FALSE(turn_reader->expect(turner::stun::binding));

		// STUN reader can check STUN messages but not TURN messages
		auto stun_reader = turner::stun::read_message(span);
		REQUIRE(stun_reader);
		CHECK_FALSE(stun_reader->expect(turner::stun::binding));
		//CHECK(stun_reader->expect(turner::turn::allocate));
	}

	SECTION("even_port_value_type") //{{{1
	{
		using message_type = test_message<turner::turn::even_port_value_type>;

		SECTION("on")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x01,
				0x80, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == true);
		}

		SECTION("off")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x01,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == false);
		}

		SECTION("accept unknown rffu bits")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x01,
				0xff, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == true);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x02,
				0x80, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("dont_fragment_value_type") //{{{1
	{
		using message_type = test_message<turner::turn::dont_fragment_value_type>;

		SECTION("on")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == true);
		}

		SECTION("off")
		{
			// attribute but found means client did not request this feature
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("channel_number_value_type") //{{{1
	{
		using message_type = test_message<turner::turn::channel_number_value_type>;

		SECTION("valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x40, 0x01, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == 0x4001);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x40, 0x01, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("below valid range")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x3f, 0xff, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("above valid range")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x80, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}
	}

	SECTION("address_error_code_value_type") //{{{1
	{
		using message_type = test_message<turner::turn::address_error_code_value_type>;

		SECTION("less")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x00, 0x00, 0x04, 0x01,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("unexpected family value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x03, 0x00, 0x04, 0x01,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("IPv4 empty reason")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x01, 0x00, 0x04, 0x01,
			};
			REQUIRE(message.value);
			auto [family, code, reason] = *message.value;
			CHECK(family == turner::address_family::v4);
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason.empty());
		}

		SECTION("IPv4 with reason")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x01, 0x00, 0x04, 0x01,
				'T',  'e',  's',  't',
			};
			REQUIRE(message.value);
			auto [family, code, reason] = *message.value;
			CHECK(family == turner::address_family::v4);
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason == "Test");
		}

		SECTION("IPv6 empty reason")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x02, 0x00, 0x04, 0x01,
			};
			REQUIRE(message.value);
			auto [family, code, reason] = *message.value;
			CHECK(family == turner::address_family::v6);
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason.empty());
		}

		SECTION("IPv6 with reason")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x02, 0x00, 0x04, 0x01,
				'T',  'e',  's',  't',
			};
			REQUIRE(message.value);
			auto [family, code, reason] = *message.value;
			CHECK(family == turner::address_family::v6);
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason == "Test");
		}
	}

	//}}}1
}


} // namespace
