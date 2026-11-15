#include <turner/codec/error.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include <system_error>
#include <utility>

namespace
{

TEST_CASE("error")
{
#define __turner_errc_value(Code, Message) turner::errc::Code,

	SECTION("errc")
	{
		const auto e = GENERATE(values({__turner_errc(__turner_errc_value)}));
		const std::error_code ec = e;
		CAPTURE(ec);
		CHECK(ec == e);
		CHECK(ec.value() == std::to_underlying(e));
		CHECK_FALSE(ec.message().empty());
		CHECK(ec.message() != "unknown");
		CHECK(ec.category() == turner::error_category());
		CHECK(ec.category().name() == std::string{"turner"});
	}

	SECTION("unknown")
	{
		const std::error_code ec = static_cast<turner::errc>(-1);
		CHECK(ec.message() == "unknown");
		CHECK(ec.category() == turner::error_category());
		CHECK(ec.category().name() == std::string{"turner"});
	}

#undef __turner_errc_value
}

} // namespace
