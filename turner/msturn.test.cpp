#include <turner/msturn>
#include <turner/test>


namespace {

using namespace turner_test;

template <typename ValueType>
using test_message = turner_test::test_message<turner::msturn, ValueType>;


TEST_CASE("msturn")
{
	SECTION("method registry") //{{{1
	{
		CHECK(turner::msturn::allocate == 0x0003_u16);
		CHECK(turner::msturn::allocate_success == 0x0103_u16);
		CHECK(turner::msturn::allocate_error == 0x0113_u16);
		CHECK(turner::msturn::send_indication == 0x0004_u16);
		CHECK(turner::msturn::data_indication == 0x0115_u16);
		CHECK(turner::msturn::set_active_destination == 0x0006_u16);
		CHECK(turner::msturn::set_active_destination_success == 0x0106_u16);
		CHECK(turner::msturn::set_active_destination_error == 0x0116_u16);
	}

	SECTION("attribute registry") //{{{1
	{
		CHECK(turner::msturn::mapped_address == 0x0001_u16);
		CHECK(turner::msturn::username == 0x0006_u16);
		CHECK(turner::msturn::message_integrity == 0x0008_u16);
		CHECK(turner::msturn::error_code == 0x0009_u16);
		CHECK(turner::msturn::unknown_attributes == 0x000a_u16);
		CHECK(turner::msturn::lifetime == 0x000d_u16);
		CHECK(turner::msturn::alternate_server == 0x000e_u16);
		CHECK(turner::msturn::bandwidth == 0x0010_u16);
		CHECK(turner::msturn::destination_address == 0x0011_u16);
		CHECK(turner::msturn::remote_address == 0x0012_u16);
		CHECK(turner::msturn::data == 0x0013_u16);
		CHECK(turner::msturn::nonce == 0x0014_u16);
		CHECK(turner::msturn::realm == 0x0015_u16);
		CHECK(turner::msturn::requested_address_family == 0x0017_u16);
		CHECK(turner::msturn::ms_version == 0x8008_u16);
		CHECK(turner::msturn::xor_mapped_address == 0x8020_u16);
		CHECK(turner::msturn::ms_alternate_host_name == 0x8032_u16);
		CHECK(turner::msturn::app_id == 0x8037_u16);
		CHECK(turner::msturn::secure_tag == 0x8039_u16);
		CHECK(turner::msturn::ms_sequence_number == 0x8050_u16);
		CHECK(turner::msturn::ms_service_quality == 0x8055_u16);
		CHECK(turner::msturn::ms_alternate_mapped_address == 0x8090_u16);
		CHECK(turner::msturn::multiplexed_session_id == 0x8095_u16);
	}

	SECTION("ms_version_value_type") //{{{1
	{
		using message_type = test_message<turner::msturn::protocol_version_value_type>;

		SECTION("valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x06,
			};
			REQUIRE(message.value);
			CHECK(*message.value == turner::msturn::protocol_version::v6);
			CHECK(*message.value >= turner::msturn::protocol_version::with_ice);
		}

		SECTION("unexpected attribute length")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x03,
				0x00, 0x00, 0x00, 0x06,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_length);
		}

		SECTION("below valid range")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x00,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}

		SECTION("above valid range")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0xff,
			};
			REQUIRE(!message.value);
			CHECK(message.value.error() == turner::errc::unexpected_attribute_value);
		}
	}

	//}}}1
}


} // namespace
