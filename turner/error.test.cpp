#include <turner/error>
#include <turner/test>
#include <catch2/generators/catch_generators_all.hpp>
#include <type_traits>


namespace {


TEST_CASE("error")
{
	#define __turner_errc_impl(code, message) turner::errc::code,

	SECTION("errc")
	{
		std::error_code ec = GENERATE(values({__turner_errc(__turner_errc_impl)}));
		CAPTURE(ec);

		SECTION("message")
		{
			CHECK_FALSE(ec.message().empty());
			CHECK(ec.message() != "unknown");
			CHECK(ec.category() == turner::error_category());
			CHECK(ec.category().name() == std::string{"turner"});
		}
	}

	SECTION("message_bad_alloc")
	{
		std::error_code ec = turner::errc::__0;
		turner_test::bad_alloc_once x;
		CHECK_THROWS_AS(ec.message(), std::bad_alloc);
	}

	SECTION("unknown")
	{
		std::error_code ec = static_cast<turner::errc>(
			std::numeric_limits<std::underlying_type_t<turner::errc>>::max()
		);
		CHECK(ec.message() == "unknown");
		CHECK(ec.category() == turner::error_category());
		CHECK(ec.category().name() == std::string{"turner"});
	}
}


} // namespace
