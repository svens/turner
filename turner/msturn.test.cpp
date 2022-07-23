#include <turner/msturn>
#include <turner/test>
#include <catch2/generators/catch_generators.hpp>


namespace {

using namespace turner_test;


TEST_CASE("msturn")
{
	SECTION("method registry")
	{
		CHECK(turner::msturn::allocate == 0x0003);
		CHECK(turner::msturn::allocate_success == 0x0103);
		CHECK(turner::msturn::allocate_error == 0x0113);
		CHECK(turner::msturn::send_request == 0x0004);
		CHECK(turner::msturn::data_indication == 0x0115);
		CHECK(turner::msturn::set_active_destination == 0x0006);
		CHECK(turner::msturn::set_active_destination_success == 0x0106);
		CHECK(turner::msturn::set_active_destination_error == 0x0116);
	}

	SECTION("attribute registry")
	{
		CHECK(turner::msturn::mapped_address == 0x0001);
		CHECK(turner::msturn::username == 0x0006);
		CHECK(turner::msturn::message_integrity == 0x0008);
		CHECK(turner::msturn::error_code == 0x0009);
		CHECK(turner::msturn::unknown_attributes == 0x000a);
		CHECK(turner::msturn::lifetime == 0x000d);
		CHECK(turner::msturn::alternate_server == 0x000e);
		CHECK(turner::msturn::bandwidth == 0x0010);
		CHECK(turner::msturn::destination_address == 0x0011);
		CHECK(turner::msturn::remote_address == 0x0012);
		CHECK(turner::msturn::data == 0x0013);
		CHECK(turner::msturn::nonce == 0x0014);
		CHECK(turner::msturn::realm == 0x0015);
		CHECK(turner::msturn::requested_address_family == 0x0017);
		CHECK(turner::msturn::ms_version == 0x8008);
		CHECK(turner::msturn::xor_mapped_address == 0x8020);
		CHECK(turner::msturn::ms_alternate_host_name == 0x8032);
		CHECK(turner::msturn::app_id == 0x8037);
		CHECK(turner::msturn::secure_tag == 0x8039);
		CHECK(turner::msturn::ms_sequence_number == 0x8050);
		CHECK(turner::msturn::ms_service_quality == 0x8055);
		CHECK(turner::msturn::ms_alternate_mapped_address == 0x8090);
		CHECK(turner::msturn::multiplexed_session_id == 0x8095);
	}
}


} // namespace
