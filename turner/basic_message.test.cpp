#include <turner/basic_message>
#include <turner/test>


namespace {

using namespace turner_test;


struct Protocol
{
	using request = turner::basic_request<Protocol, 0x0001>;
	using ok = turner::basic_success_response<request>;
	using not_ok = turner::basic_error_response<request>;
	using signal = turner::basic_indication<Protocol, 0x0002>;
};


TEST_CASE("basic_message")
{
	CHECK(std::is_same_v<Protocol::request::protocol_type, Protocol>);
	CHECK(Protocol::request::method == 0x0001);
	CHECK(Protocol::request::type == 0x0001);
	CHECK(Protocol::request::is_request);
	CHECK_FALSE(Protocol::request::is_success_response);
	CHECK_FALSE(Protocol::request::is_error_response);
	CHECK_FALSE(Protocol::request::is_indication);

	CHECK(std::is_same_v<Protocol::ok::protocol_type, Protocol>);
	CHECK(Protocol::ok::method == 0x0001);
	CHECK(Protocol::ok::type == 0x0101);
	CHECK_FALSE(Protocol::ok::is_request);
	CHECK(Protocol::ok::is_success_response);
	CHECK_FALSE(Protocol::ok::is_error_response);
	CHECK_FALSE(Protocol::ok::is_indication);

	CHECK(std::is_same_v<Protocol::not_ok::protocol_type, Protocol>);
	CHECK(Protocol::not_ok::method == 0x0001);
	CHECK(Protocol::not_ok::type == 0x0111);
	CHECK_FALSE(Protocol::not_ok::is_request);
	CHECK_FALSE(Protocol::not_ok::is_success_response);
	CHECK(Protocol::not_ok::is_error_response);
	CHECK_FALSE(Protocol::not_ok::is_indication);

	CHECK(std::is_same_v<Protocol::signal::protocol_type, Protocol>);
	CHECK(Protocol::signal::method == 0x0002);
	CHECK(Protocol::signal::type == 0x0012);
	CHECK_FALSE(Protocol::signal::is_request);
	CHECK_FALSE(Protocol::signal::is_success_response);
	CHECK_FALSE(Protocol::signal::is_error_response);
	CHECK(Protocol::signal::is_indication);
}


} // namespace
