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
		static_assert(turner::msturn::allocate.method == 0x0003);
		static_assert(turner::msturn::allocate.type == 0x0003);
		static_assert(turner::msturn::allocate.success.type == 0x0103);
		static_assert(turner::msturn::allocate.error.type == 0x0113);

		static_assert(turner::msturn::send_request.method == 0x0004);
		static_assert(turner::msturn::send_request.type == 0x0004);

		static_assert(turner::msturn::data_indication.method == 0x0115);
		static_assert(turner::msturn::data_indication.type == 0x0115);

		static_assert(turner::msturn::set_active_destination.method == 0x0006);
		static_assert(turner::msturn::set_active_destination.type == 0x0006);
		static_assert(turner::msturn::set_active_destination.success.type == 0x0106);
		static_assert(turner::msturn::set_active_destination.error.type == 0x0116);
	}

	SECTION("attribute registry") //{{{1
	{
		static_assert(turner::msturn::mapped_address == 0x0001);
		static_assert(turner::msturn::username == 0x0006);
		static_assert(turner::msturn::message_integrity == 0x0008);
		static_assert(turner::msturn::error_code == 0x0009);
		static_assert(turner::msturn::unknown_attributes == 0x000a);
		static_assert(turner::msturn::lifetime == 0x000d);
		static_assert(turner::msturn::alternate_server == 0x000e);
		static_assert(turner::msturn::bandwidth == 0x0010);
		static_assert(turner::msturn::destination_address == 0x0011);
		static_assert(turner::msturn::remote_address == 0x0012);
		static_assert(turner::msturn::data == 0x0013);
		static_assert(turner::msturn::nonce == 0x0014);
		static_assert(turner::msturn::realm == 0x0015);
		static_assert(turner::msturn::requested_address_family == 0x0017);
		static_assert(turner::msturn::ms_version == 0x8008);
		static_assert(turner::msturn::xor_mapped_address == 0x8020);
		static_assert(turner::msturn::ms_alternate_host_name == 0x8032);
		static_assert(turner::msturn::app_id == 0x8037);
		static_assert(turner::msturn::secure_tag == 0x8039);
		static_assert(turner::msturn::ms_sequence_number == 0x8050);
		static_assert(turner::msturn::ms_service_quality == 0x8055);
		static_assert(turner::msturn::ms_alternate_mapped_address == 0x8090);
		static_assert(turner::msturn::multiplexed_session_id == 0x8095);
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
