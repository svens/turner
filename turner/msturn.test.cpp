#include <turner/msturn>
#include <turner/test>


namespace {

using namespace turner_test;
using turner::msturn;

template <typename ValueType>
using test_message = turner_test::test_message<msturn, ValueType>;


TEST_CASE("msturn")
{
	SECTION("method registry") //{{{1
	{
		static_assert(msturn::allocate.method == 0x0003);
		static_assert(msturn::allocate.type == 0x0003);
		static_assert(msturn::allocate.success.type == 0x0103);
		static_assert(msturn::allocate.error.type == 0x0113);

		static_assert(msturn::send_request.method == 0x0004);
		static_assert(msturn::send_request.type == 0x0004);

		static_assert(msturn::data_indication.method == 0x0115);
		static_assert(msturn::data_indication.type == 0x0115);

		static_assert(msturn::set_active_destination.method == 0x0006);
		static_assert(msturn::set_active_destination.type == 0x0006);
		static_assert(msturn::set_active_destination.success.type == 0x0106);
		static_assert(msturn::set_active_destination.error.type == 0x0116);
	}

	SECTION("attribute registry") //{{{1
	{
		static_assert(msturn::mapped_address.type == 0x0001);
		static_assert(msturn::username.type == 0x0006);
		static_assert(msturn::message_integrity.type == 0x0008);
		static_assert(msturn::error_code.type == 0x0009);
		static_assert(msturn::unknown_attributes.type == 0x000a);
		static_assert(msturn::lifetime.type == 0x000d);
		static_assert(msturn::alternate_server.type == 0x000e);
		static_assert(msturn::bandwidth.type == 0x0010);
		static_assert(msturn::destination_address.type == 0x0011);
		static_assert(msturn::remote_address.type == 0x0012);
		static_assert(msturn::data.type == 0x0013);
		static_assert(msturn::nonce.type == 0x0014);
		static_assert(msturn::realm.type == 0x0015);
		static_assert(msturn::requested_address_family.type == 0x0017);
		static_assert(msturn::ms_version.type == 0x8008);
		static_assert(msturn::xor_mapped_address.type == 0x8020);
		static_assert(msturn::ms_alternate_host_name.type == 0x8032);
		static_assert(msturn::app_id.type == 0x8037);
		static_assert(msturn::secure_tag.type == 0x8039);
		// TODO static_assert(msturn::ms_sequence_number.type == 0x8050);
		// TODO static_assert(msturn::ms_service_quality.type == 0x8055);
		static_assert(msturn::ms_alternate_mapped_address.type == 0x8090);
		static_assert(msturn::multiplexed_session_id.type == 0x8095);
	}

	SECTION("ms_version_value_type") //{{{1
	{
		using message_type = test_message<msturn::protocol_version_value_type>;

		SECTION("valid")
		{
			message_type message
			{
				0x80, 0x80, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x06,
			};
			REQUIRE(message.value);
			CHECK(*message.value == msturn::protocol_version::v6);
			CHECK(*message.value >= msturn::protocol_version::with_ice);
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
