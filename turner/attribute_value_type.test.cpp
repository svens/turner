#include <turner/attribute_value_type>
#include <turner/msturn>
#include <turner/stun>
#include <turner/turn>
#include <turner/test>
#include <catch2/catch_template_test_macros.hpp>


namespace {


struct msturn //{{{1
{
	using protocol_type = turner::msturn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x28, // MS-TURN Allocation
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
	};
};


struct stun //{{{1
{
	using protocol_type = turner::stun;

	static constexpr uint8_t message[] =
	{
		0x00, 0x01, 0x00, 0x20, // STUN Binding
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
	};
};


struct turn //{{{1
{
	using protocol_type = turner::turn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x20, // TURN Allocate
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

	SECTION("uint32_value_type")
	{
		using attribute_type = turner::attribute_type<Protocol, turner::uint32_value_type>;

		SECTION("valid")
		{
			static constexpr attribute_type attribute = 0x8080;
			CHECK(reader.read(attribute) == 1u);
		}

		SECTION("unexpected attribute length")
		{
			static constexpr attribute_type attribute = 0x8081;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}
	}

	SECTION("seconds_value_type")
	{
		using namespace std::chrono_literals;
		using attribute_type = turner::attribute_type<Protocol, turner::seconds_value_type>;

		SECTION("valid")
		{
			static constexpr attribute_type attribute = 0x8082;
			CHECK(reader.read(attribute) == 2min);
		}

		SECTION("unexpected attribute length")
		{
			static constexpr attribute_type attribute = 0x8083;
			auto value = reader.read(attribute);
			REQUIRE(!value);
			CHECK(value.error() == turner::errc::unexpected_attribute_length);
		}
	}
}


} // namespace
