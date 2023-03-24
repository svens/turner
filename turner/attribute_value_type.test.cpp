#include <turner/attribute_value_type>
#include <turner/msturn>
#include <turner/stun>
#include <turner/turn>
#include <turner/test>
#include <catch2/catch_template_test_macros.hpp>

namespace {

using namespace turner_test;

using turner::msturn;
using turner::stun;
using turner::turn;

TEMPLATE_TEST_CASE("attribute_value_type", "",
	msturn,
	stun,
	turn)
{
	SECTION("uint32_value_type") //{{{{1
	{
		using message_type = test_message<TestType, turner::uint32_value_type>;

		SECTION("valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x01,
			};
			REQUIRE(message.value);
			CHECK(*message.value == 1u);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x00, 0x00, 0x00, 0x01,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("seconds_value_type") //{{{1
	{
		using namespace std::chrono_literals;
		using message_type = test_message<TestType, turner::seconds_value_type>;

		SECTION("valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x78,
			};
			REQUIRE(message.value);
			CHECK(*message.value == 2min);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x00, 0x00, 0x00, 0x78,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("address_family_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::address_family_value_type>;

		SECTION("IPv4 valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x01, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == turner::address_family::v4);
		}

		SECTION("IPv6 valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x02, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == turner::address_family::v6);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x02, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("unexpected attribute value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0xff, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}
	}

	SECTION("transport_protocol_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::transport_protocol_value_type>;

		SECTION("UDP valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x11, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == turner::transport_protocol::udp);
		}

		SECTION("TCP valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x06, 0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == turner::transport_protocol::tcp);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x11, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("unexpected attribute value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x01, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}
	}

	SECTION("string_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::string_value_type<4>>;

		SECTION("less")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				't',  'e',  's',  0x00,
			};
			REQUIRE(message.value);
			CHECK(*message.value == "tes");
		}

		SECTION("exact")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				't',  'e',  's',  't',
			};
			REQUIRE(message.value);
			CHECK(*message.value == "test");
		}

		SECTION("exceed")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x05,
				't',  'e',  's',  't',
				't',  0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("empty")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(message.value->empty());
		}
	}

	SECTION("bytes_value_type<std::dynamic_extent>") //{{{1
	{
		using message_type = test_message<TestType, turner::bytes_value_type<>>;

		SECTION("less")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				't',  'e',  's',  0x00,
			};
			static const auto expected_value = "tes"_b;
			REQUIRE(message.value);
			auto &value = *message.value;
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("exact")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				't',  'e',  's',  't',
			};
			REQUIRE(message.value);
			auto &value = *message.value;
			static const auto expected_value = "test"_b;
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("exceed")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x05,
				't',  'e',  's',  't',
				't',  0x00, 0x00, 0x00,
			};
			REQUIRE(message.value);
			auto &value = *message.value;
			static const auto expected_value = "testt"_b;
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("empty")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(message.value);
			CHECK(message.value->empty());
		}
	}

	SECTION("bytes_value_type<4>") //{{{1
	{
		using message_type = test_message<TestType, turner::bytes_value_type<4>>;

		SECTION("less")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				't',  'e',  's',  0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("exact")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				't',  'e',  's',  't',
			};
			REQUIRE(message.value);
			auto &value = *message.value;
			static const auto expected_value = "test"_b;
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("exceed")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x05,
				't',  'e',  's',  't',
				't',  0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("empty")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("error_code_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::error_code_value_type>;

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

		SECTION("empty reason")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x04, 0x01,
			};
			REQUIRE(message.value);
			auto [code, reason] = *message.value;
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason.empty());
		}

		SECTION("with reason")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x00, 0x00, 0x04, 0x01,
				'T',  'e',  's',  't',
			};
			REQUIRE(message.value);
			auto [code, reason] = *message.value;
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason == "Test");
		}
	}

	SECTION("attribute_list_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::attribute_list_value_type>;

		SECTION("less")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x80, 0x8b, 0x10, 0x01,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("exact")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x06,
				0x80, 0x8b, 0x80, 0x8c,
				0x80, 0x8d, 0x00, 0x00,
			};
			REQUIRE(message.value);
			auto [size, value] = *message.value;
			REQUIRE(size == 3);
			static const std::array<uint16_t, 4> expected_value = { 0x808b, 0x808c, 0x808d };
			CHECK(value == expected_value);
		}

		SECTION("exceed")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x0a,
				0x80, 0x8b, 0x80, 0x8c,
				0x80, 0x8d, 0x80, 0x8e,
				0x80, 0x8f, 0x00, 0x00,
			};
			REQUIRE(message.value);
			auto [size, value] = *message.value;
			REQUIRE(size == 5);
			static const std::array<uint16_t, 4> expected_value = { 0x808b, 0x808c, 0x808d, 0x808e };
			CHECK(value == expected_value);
		}

		SECTION("empty")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(message.value);
			auto [size, value] = *message.value;
			REQUIRE(size == 0);
			static const std::array<uint16_t, 4> expected_value = {};
			CHECK(value == expected_value);
		}
	}

	SECTION("endpoint_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::endpoint_value_type<TestType>>;

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("IPv4 unexpected attribute value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x00, 0xff, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("IPv6 unexpected attribute value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x14,
				0x00, 0xff, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("IPv4 success")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x00, 0x01, 0x12, 0x34,
				0x7f, 0x00, 0x00, 0x01,
			};
			REQUIRE(message.value);
			auto [address, port] = *message.value;
			CHECK(address == pal::net::ip::address_v4::loopback());
			CHECK(port == 0x1234);
		}

		SECTION("IPv6 success")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x14,
				0x00, 0x02, 0x23, 0x45,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x01,
			};
			REQUIRE(message.value);
			auto [address, port] = *message.value;
			CHECK(address == pal::net::ip::address_v6::loopback());
			CHECK(port == 0x2345);
		}
	}

	SECTION("xor_endpoint_value_type") //{{{1
	{
		using message_type = test_message<TestType, turner::xor_endpoint_value_type<TestType>>;

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("IPv4 unexpected attribute value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x00, 0xff, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("IPv6 unexpected attribute value")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x14,
				0x00, 0xff, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("IPv4 success")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x08,
				0x00, 0x01, 0x33, 0x26,
				0x5e, 0x12, 0xa4, 0x43,
			};
			REQUIRE(message.value);
			auto [address, port] = *message.value;
			CHECK(address == pal::net::ip::address_v4::loopback());
			CHECK(port == 0x1234);
		}

		SECTION("IPv6 success")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x14,
				0x00, 0x02, 0x02, 0x57,
				0x21, 0x12, 0xa4, 0x42,
				0x00, 0x01, 0x02, 0x03,
				0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0a, 0x0a,
			};
			REQUIRE(message.value);
			auto [address, port] = *message.value;
			CHECK(address == pal::net::ip::address_v6::loopback());
			CHECK(port == 0x2345);
		}
	}

	//}}}1
}

} // namespace
