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
	static_assert(std::is_same_v<typename decltype(Protocol::request)::protocol_type, Protocol>);
	static_assert(Protocol::request.method == 0x0001);
	static_assert(Protocol::request.type == 0x0001);
	static_assert(Protocol::request.is_request);
	static_assert(!Protocol::request.is_success_response);
	static_assert(!Protocol::request.is_error_response);
	static_assert(!Protocol::request.is_indication);

	// request success response
	static_assert(std::is_same_v<typename decltype(Protocol::ok)::protocol_type, Protocol>);
	static_assert(Protocol::ok.method == 0x0001);
	static_assert(Protocol::ok.type == 0x0101);
	static_assert(!Protocol::ok.is_request);
	static_assert(Protocol::ok.is_success_response);
	static_assert(!Protocol::ok.is_error_response);
	static_assert(!Protocol::ok.is_indication);

	// request error response
	static_assert(std::is_same_v<typename decltype(Protocol::not_ok)::protocol_type, Protocol>);
	static_assert(Protocol::not_ok.method == 0x0001);
	static_assert(Protocol::not_ok.type == 0x0111);
	static_assert(!Protocol::not_ok.is_request);
	static_assert(!Protocol::not_ok.is_success_response);
	static_assert(Protocol::not_ok.is_error_response);
	static_assert(!Protocol::not_ok.is_indication);

	static_assert(std::is_same_v<typename decltype(Protocol::signal)::protocol_type, Protocol>);
	static_assert(Protocol::signal.method == 0x0002);
	static_assert(Protocol::signal.type == 0x0012);
	static_assert(!Protocol::signal.is_request);
	static_assert(!Protocol::signal.is_success_response);
	static_assert(!Protocol::signal.is_error_response);
	static_assert(Protocol::signal.is_indication);
}

} // namespace
