#include <turner/attribute_value_type>
#include <turner/msturn>
#include <turner/stun>
#include <turner/turn>
#include <turner/test>
#include <catch2/catch_template_test_macros.hpp>


namespace {


using namespace turner_test;


struct msturn //{{{1
{
	using protocol_type = turner::msturn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x8c, // MS-TURN Allocation
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,

		0x00, 0x0f, 0x00, 0x04, // Magic Cookie
		0x72, 0xc6, 0x4b, 0xc6,

		0x80, 0x80, 0x00, 0x04, // 0x8080, uint32_value_type
		0x00, 0x00, 0x00, 0x01, // 1

		0x80, 0x81, 0x00, 0x03, // 0x8081, uint32_value_type
		0x00, 0x00, 0x00, 0x01, // unexpected attribute length

		0x80, 0x82, 0x00, 0x04, // 0x8082, seconds_value_type
		0x00, 0x00, 0x00, 0x78, // 120sec

		0x80, 0x83, 0x00, 0x03, // 0x8083, seconds_value_type
		0x00, 0x00, 0x00, 0x01, // unexpected attribute length

		0x80, 0x84, 0x00, 0x03, // 0x8084, string_value_type<4>
		't',  'e',  's',  0x00, // less

		0x80, 0x85, 0x00, 0x04, // 0x8085, string_value_type<4>
		't',  'e',  's',  't',  // exact

		0x80, 0x86, 0x00, 0x05, // 0x8086, string_value_type<4>
		't',  'e',  's',  't',  // exceed
		't',  0x00, 0x00, 0x00,

		0x80, 0x87, 0x00, 0x00, // 0x8087, string_value_type<4>

		0x80, 0x88, 0x00, 0x03, // 0x8088, error_code_value_type
		0x00, 0x00, 0x04, 0x01, // less

		0x80, 0x89, 0x00, 0x04, // 0x8089, error_code_value_type
		0x00, 0x00, 0x04, 0x01, // exact size, empty reason

		0x80, 0x8a, 0x00, 0x08, // 0x8089, error_code_value_type
		0x00, 0x00, 0x04, 0x01,
		'T',  'e',  's',  't',

		0x80, 0x8b, 0x00, 0x03, // 0x808b, attribute_list_value_type
		0x80, 0x8b, 0x10, 0x01, // less

		0x80, 0x8c, 0x00, 0x06, // 0x808c, attribute_list_value_type
		0x80, 0x8b, 0x80, 0x8c, // exact (3 elments)
		0x80, 0x8d, 0x10, 0x01,

		0x80, 0x8d, 0x00, 0x0a, // 0x808d, attribute_list_value_type
		0x80, 0x8b, 0x80, 0x8c, // exceed (4 allowed, 5 provided)
		0x80, 0x8d, 0x80, 0x8e,
		0xff, 0xff, 0x00, 0x00,

		0x80, 0x8e, 0x00, 0x00, // 0x808e, attribute_list_value_type
	};
};


struct stun //{{{1
{
	using protocol_type = turner::stun;

	static constexpr uint8_t message[] =
	{
		0x00, 0x01, 0x00, 0x84, // STUN Binding
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,

		0x80, 0x80, 0x00, 0x04, // 0x8080, uint32_value_type
		0x00, 0x00, 0x00, 0x01, // 1

		0x80, 0x81, 0x00, 0x03, // 0x8081, uint32_value_type
		0x00, 0x00, 0x00, 0x01, // unexpected attribute length

		0x80, 0x82, 0x00, 0x04, // 0x8082, seconds_value_type
		0x00, 0x00, 0x00, 0x78, // 120sec

		0x80, 0x83, 0x00, 0x03, // 0x8083, seconds_value_type
		0x00, 0x00, 0x00, 0x01, // unexpected attribute length

		0x80, 0x84, 0x00, 0x03, // 0x8084, string_value_type<4>
		't',  'e',  's',  0x00, // less

		0x80, 0x85, 0x00, 0x04, // 0x8085, string_value_type<4>
		't',  'e',  's',  't',  // exact

		0x80, 0x86, 0x00, 0x05, // 0x8086, string_value_type<4>
		't',  'e',  's',  't',  // exceed
		't',  0x00, 0x00, 0x00,

		0x80, 0x87, 0x00, 0x00, // 0x8087, string_value_type<4>

		0x80, 0x88, 0x00, 0x03, // 0x8088, error_code_value_type
		0x00, 0x00, 0x04, 0x01, // less

		0x80, 0x89, 0x00, 0x04, // 0x8089, error_code_value_type
		0x00, 0x00, 0x04, 0x01, // exact size, empty reason

		0x80, 0x8a, 0x00, 0x08, // 0x8089, error_code_value_type
		0x00, 0x00, 0x04, 0x01,
		'T',  'e',  's',  't',

		0x80, 0x8b, 0x00, 0x03, // 0x808b, attribute_list_value_type
		0x80, 0x8b, 0x10, 0x01, // less

		0x80, 0x8c, 0x00, 0x06, // 0x808c, attribute_list_value_type
		0x80, 0x8b, 0x80, 0x8c, // exact (3 elments)
		0x80, 0x8d, 0x10, 0x01,

		0x80, 0x8d, 0x00, 0x0a, // 0x808d, attribute_list_value_type
		0x80, 0x8b, 0x80, 0x8c, // exceed (4 allowed, 5 provided)
		0x80, 0x8d, 0x80, 0x8e,
		0xff, 0xff, 0x00, 0x00,

		0x80, 0x8e, 0x00, 0x00, // 0x808e, attribute_list_value_type
	};
};


struct turn //{{{1
{
	using protocol_type = turner::turn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x84, // TURN Allocate
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,

		0x80, 0x80, 0x00, 0x04, // 0x8080, uint32_value_type
		0x00, 0x00, 0x00, 0x01, // 1

		0x80, 0x81, 0x00, 0x03, // 0x8081, uint32_value_type
		0x00, 0x00, 0x00, 0x01, // unexpected attribute length

		0x80, 0x82, 0x00, 0x04, // 0x8082, seconds_value_type
		0x00, 0x00, 0x00, 0x78, // 120sec

		0x80, 0x83, 0x00, 0x03, // 0x8083, seconds_value_type
		0x00, 0x00, 0x00, 0x01, // unexpected attribute length

		0x80, 0x84, 0x00, 0x03, // 0x8084, string_value_type<4> || bytes_value_type<>
		't',  'e',  's',  0x00, // less

		0x80, 0x85, 0x00, 0x04, // 0x8085, string_value_type<4> || bytes_value_type<>
		't',  'e',  's',  't',  // exact

		0x80, 0x86, 0x00, 0x05, // 0x8086, string_value_type<4> || bytes_value_type<>
		't',  'e',  's',  't',  // exceed
		't',  0x00, 0x00, 0x00,

		0x80, 0x87, 0x00, 0x00, // 0x8087, string_value_type<4> || bytes_value_type<>

		0x80, 0x88, 0x00, 0x03, // 0x8088, error_code_value_type
		0x00, 0x00, 0x04, 0x01, // less

		0x80, 0x89, 0x00, 0x04, // 0x8089, error_code_value_type
		0x00, 0x00, 0x04, 0x01, // exact size, empty reason

		0x80, 0x8a, 0x00, 0x08, // 0x8089, error_code_value_type
		0x00, 0x00, 0x04, 0x01,
		'T',  'e',  's',  't',

		0x80, 0x8b, 0x00, 0x03, // 0x808b, attribute_list_value_type
		0x80, 0x8b, 0x10, 0x01, // less

		0x80, 0x8c, 0x00, 0x06, // 0x808c, attribute_list_value_type
		0x80, 0x8b, 0x80, 0x8c, // exact (3 elments)
		0x80, 0x8d, 0x10, 0x01,

		0x80, 0x8d, 0x00, 0x0a, // 0x808d, attribute_list_value_type
		0x80, 0x8b, 0x80, 0x8c, // exceed (4 allowed, 5 provided)
		0x80, 0x8d, 0x80, 0x8e,
		0xff, 0xff, 0x00, 0x00,

		0x80, 0x8e, 0x00, 0x00, // 0x808e, attribute_list_value_type
	};
};

//}}}1


TEMPLATE_TEST_CASE("attribute_value_type", "",
	msturn,
	stun,
	turn)
{
	using Protocol = typename TestType::protocol_type;
	auto span = std::as_bytes(std::span{TestType::message});
	auto reader = pal_try(Protocol::read_message(span));

	SECTION("uint32_value_type") //{{{1
	{
		using attribute_type = turner::attribute_type<Protocol, turner::uint32_value_type>;

		SECTION("valid")
		{
			static constexpr attribute_type attribute = 0x8080;
			CHECK(pal_try(reader.read(attribute)) == 1u);
		}

		SECTION("unexpected attribute length")
		{
			static constexpr attribute_type attribute = 0x8081;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("seconds_value_type") //{{{1
	{
		using namespace std::chrono_literals;
		using attribute_type = turner::attribute_type<Protocol, turner::seconds_value_type>;

		SECTION("valid")
		{
			static constexpr attribute_type attribute = 0x8082;
			CHECK(pal_try(reader.read(attribute)) == 2min);
		}

		SECTION("unexpected attribute length")
		{
			static constexpr attribute_type attribute = 0x8083;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("string_value_type") //{{{1
	{
		using attribute_type = turner::attribute_type<Protocol, turner::string_value_type<4>>;

		SECTION("less")
		{
			static constexpr attribute_type attribute = 0x8084;
			CHECK(pal_try(reader.read(attribute)) == "tes");
		}

		SECTION("exact")
		{
			static constexpr attribute_type attribute = 0x8085;
			CHECK(pal_try(reader.read(attribute)) == "test");
		}

		SECTION("exceed")
		{
			static constexpr attribute_type attribute = 0x8086;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("empty")
		{
			static constexpr attribute_type attribute = 0x8087;
			CHECK(pal_try(reader.read(attribute)).empty());
		}
	}

	SECTION("bytes_value_type<std::dynamic_extent>") //{{{1
	{
		using attribute_type = turner::attribute_type<Protocol, turner::bytes_value_type<>>;

		SECTION("less")
		{
			static constexpr attribute_type attribute = 0x8084;
			static const auto expected_value = "tes"_b;
			auto value = pal_try(reader.read(attribute));
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("exact")
		{
			static constexpr attribute_type attribute = 0x8085;
			static const auto expected_value = "test"_b;
			auto value = pal_try(reader.read(attribute));
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("exceed")
		{
			static constexpr attribute_type attribute = 0x8086;
			static const auto expected_value = "testt"_b;
			auto value = pal_try(reader.read(attribute));
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("empty")
		{
			static constexpr attribute_type attribute = 0x8087;
			CHECK(pal_try(reader.read(attribute)).empty());
		}
	}

	SECTION("bytes_value_type<4>") //{{{1
	{
		using attribute_type = turner::attribute_type<Protocol, turner::bytes_value_type<4>>;

		SECTION("less")
		{
			static constexpr attribute_type attribute = 0x8084;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("exact")
		{
			static constexpr attribute_type attribute = 0x8085;
			static const auto expected_value = "test"_b;
			auto value = pal_try(reader.read(attribute));
			CHECK(std::equal(value.begin(), value.end(), expected_value.begin(), expected_value.end()));
		}

		SECTION("exceed")
		{
			static constexpr attribute_type attribute = 0x8086;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("empty")
		{
			static constexpr attribute_type attribute = 0x8087;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("error_code_value_type") //{{{1
	{
		using attribute_type = turner::attribute_type<Protocol, turner::error_code_value_type>;

		SECTION("less")
		{
			static constexpr attribute_type attribute = 0x8088;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("empty reason")
		{
			static constexpr attribute_type attribute = 0x8089;
			auto [code, reason] = pal_try(reader.read(attribute));
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason.empty());
		}

		SECTION("with reason")
		{
			static constexpr attribute_type attribute = 0x808a;
			auto [code, reason] = pal_try(reader.read(attribute));
			CHECK(code == turner::protocol_errc::unauthorized);
			CHECK(reason == "Test");
		}
	}

	SECTION("attribute_list_value_type") //{{{1
	{
		using attribute_type = turner::attribute_type<Protocol, turner::attribute_list_value_type>;

		SECTION("less")
		{
			static constexpr attribute_type attribute = 0x808b;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("exact")
		{
			static constexpr attribute_type attribute = 0x808c;
			static const std::array<uint16_t, 4> expected_value = { 0x808b, 0x808c, 0x808d };
			auto [size, value] = pal_try(reader.read(attribute));
			REQUIRE(size == 3);
			CHECK(value == expected_value);
		}

		SECTION("exceed")
		{
			static constexpr attribute_type attribute = 0x808d;
			static const std::array<uint16_t, 4> expected_value = { 0x808b, 0x808c, 0x808d, 0x808e };
			auto [size, value] = pal_try(reader.read(attribute));
			REQUIRE(size == 5);
			CHECK(value == expected_value);
		}

		SECTION("empty")
		{
			static constexpr attribute_type attribute = 0x808e;
			static const std::array<uint16_t, 4> expected_value = {};
			auto [size, value] = pal_try(reader.read(attribute));
			CHECK(size == 0);
			CHECK(value == expected_value);
		}
	}

	//}}}1
}


} // namespace
