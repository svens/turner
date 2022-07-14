#include <turner/protocol_error>
#include <turner/test>
#include <catch2/generators/catch_generators_all.hpp>
#include <type_traits>


namespace {

TEST_CASE("protocol_error")
{
	#define __turner_errc_impl(value, symbol, message) turner::protocol_errc::symbol,

	SECTION("protocol_errc")
	{
		std::error_code ec = GENERATE(values({__turner_protocol_errc(__turner_errc_impl)}));
		CAPTURE(ec);

		SECTION("message")
		{
			CHECK_FALSE(ec.message().empty());
			CHECK(ec.message() != "unknown");
			CHECK(ec.category() == turner::protocol_error_category());
			CHECK(ec.category().name() == std::string{"turner::protocol"});
		}
	}

	SECTION("unknown")
	{
		std::error_code ec = static_cast<turner::protocol_errc>(
			std::numeric_limits<std::underlying_type_t<turner::protocol_errc>>::max()
		);
		CHECK(ec.message() == "unknown");
		CHECK(ec.category() == turner::protocol_error_category());
		CHECK(ec.category().name() == std::string{"turner::protocol"});
	}
}

} // namespace
