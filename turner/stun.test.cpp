#include <turner/stun>
#include <turner/test>
#include <catch2/generators/catch_generators.hpp>


namespace {

TEST_CASE("stun")
{
	SECTION("method registry")
	{
		auto [message_type, method] = GENERATE(table<turner::message_type<turner::stun>, uint16_t>(
		{
			{ turner::stun::binding,		{0x001} },
			{ turner::stun::binding_success,	{0x101} },
		}));
		CHECK(message_type == method);
	}

	SECTION("magic cookie")
	{
		CHECK(turner::stun::magic_cookie == 0x21'12'a4'42);
	}

	SECTION("read success")
	{
		uint8_t data[32] = { 0x00, 0x01, };
		auto r = pal_try(turner::stun::read(std::as_bytes(std::span{data})));
		CAPTURE(r.message_type().value);
		CHECK(r.message_type() == turner::stun::binding);
		auto txn = r.transaction_id();
		CHECK(txn.size() == 12);
	}
}


} // namespace
