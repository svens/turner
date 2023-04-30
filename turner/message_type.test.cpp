#include <turner/message_type>
#include <turner/test>

namespace {

using namespace turner_test;

struct Protocol
{
	static constexpr auto request = turner::request<Protocol, 0x0001>;
	static constexpr auto ok = request.success;
	static constexpr auto not_ok = request.error;
	static constexpr auto signal = turner::indication<Protocol, 0x0002>;
};

TEST_CASE("message_type")
{
	// request
	__turner_check(std::is_same_v<typename decltype(Protocol::request)::protocol_type, Protocol>);
	__turner_check(Protocol::request.method == 0x0001);
	__turner_check(Protocol::request.type == 0x0001);
	__turner_check(Protocol::request == 0x0001);
	__turner_check(Protocol::request != 0xffff);
	__turner_check(Protocol::request.is_request);
	__turner_check(!Protocol::request.is_success_response);
	__turner_check(!Protocol::request.is_error_response);
	__turner_check(!Protocol::request.is_indication);

	// request success response
	__turner_check(std::is_same_v<typename decltype(Protocol::ok)::protocol_type, Protocol>);
	__turner_check(Protocol::ok.method == 0x0001);
	__turner_check(Protocol::ok.type == 0x0101);
	__turner_check(Protocol::ok == 0x0101);
	__turner_check(Protocol::ok != 0xffff);
	__turner_check(!Protocol::ok.is_request);
	__turner_check(Protocol::ok.is_success_response);
	__turner_check(!Protocol::ok.is_error_response);
	__turner_check(!Protocol::ok.is_indication);

	// request error response
	__turner_check(std::is_same_v<typename decltype(Protocol::not_ok)::protocol_type, Protocol>);
	__turner_check(Protocol::not_ok.method == 0x0001);
	__turner_check(Protocol::not_ok.type == 0x0111);
	__turner_check(Protocol::not_ok == 0x0111);
	__turner_check(Protocol::not_ok != 0xffff);
	__turner_check(!Protocol::not_ok.is_request);
	__turner_check(!Protocol::not_ok.is_success_response);
	__turner_check(Protocol::not_ok.is_error_response);
	__turner_check(!Protocol::not_ok.is_indication);

	__turner_check(std::is_same_v<typename decltype(Protocol::signal)::protocol_type, Protocol>);
	__turner_check(Protocol::signal.method == 0x0002);
	__turner_check(Protocol::signal.type == 0x0012);
	__turner_check(Protocol::signal == 0x0012);
	__turner_check(Protocol::signal != 0xffff);
	__turner_check(!Protocol::signal.is_request);
	__turner_check(!Protocol::signal.is_success_response);
	__turner_check(!Protocol::signal.is_error_response);
	__turner_check(Protocol::signal.is_indication);
}

} // namespace
