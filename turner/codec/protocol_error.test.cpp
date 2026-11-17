#include <turner/codec/protocol_error.hpp>
#include <turner/codec/error.hpp>
#include <pal/result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <concepts>
#include <string>
#include <system_error>
#include <utility>

namespace
{

static_assert(std::to_underlying(turner::protocol_errc::try_alternate) == 300);
static_assert(std::to_underlying(turner::protocol_errc::unauthorized) == 401);
static_assert(std::to_underlying(turner::protocol_errc::unknown_attribute) == 420);
static_assert(std::to_underlying(turner::protocol_errc::stale_nonce) == 438);
static_assert(std::to_underlying(turner::protocol_errc::insufficient_capacity) == 508);
static_assert(turner::reason_phrase(turner::protocol_errc::stale_nonce) == "Stale Nonce");

TEST_CASE("protocol_error")
{
#define __turner_protocol_errc_value(Value, Code, Message) turner::protocol_errc::Code,

	SECTION("protocol_errc")
	{
		const auto e = GENERATE(values({__turner_protocol_errc(__turner_protocol_errc_value)}));
		const std::error_code ec = e;
		CAPTURE(ec);
		CHECK(ec == e);
		CHECK(ec.value() == std::to_underlying(e));
		CHECK_FALSE(ec.message().empty());
		CHECK(ec.message() != "unknown");
		CHECK(ec.category() == turner::protocol_error_category());
		CHECK(ec.category().name() == std::string{"turner::protocol"});
	}

	SECTION("reason_phrase")
	{
		const auto e = GENERATE(values({__turner_protocol_errc(__turner_protocol_errc_value)}));
		const std::error_code ec = e;
		CAPTURE(ec);
		CHECK(turner::reason_phrase(e) == ec.message());
	}

	SECTION("reason_phrase: unknown")
	{
		CHECK(turner::reason_phrase(static_cast<turner::protocol_errc>(499)).empty());
	}

	SECTION("protocol_error")
	{
		constexpr turner::protocol_error bare = turner::protocol_errc::unauthorized;
		static_assert(bare.code == turner::protocol_errc::unauthorized);
		static_assert(bare.reason == "Unauthorized");

		constexpr turner::protocol_error unknown = static_cast<turner::protocol_errc>(499);
		static_assert(unknown.reason.empty());

		constexpr turner::protocol_error custom{turner::protocol_errc::unauthorized, "Go away"};
		static_assert(custom.code == turner::protocol_errc::unauthorized);
		static_assert(custom.reason == "Go away");

		static_assert(!std::equality_comparable<turner::protocol_error>);
		static_assert(custom == turner::protocol_errc::unauthorized);
		static_assert(turner::protocol_errc::unauthorized == custom);
		static_assert(custom != turner::protocol_errc::stale_nonce);
		static_assert(turner::protocol_errc::stale_nonce != custom);
	}

	SECTION("protocol_error: result compares code only")
	{
		const pal::result<turner::protocol_error> result{
			std::in_place,
			turner::protocol_errc::stale_nonce,
			"nonce expired",
		};
		CHECK(result == turner::protocol_errc::stale_nonce);
		CHECK(result != turner::protocol_errc::unauthorized);
	}

	SECTION("message")
	{
		const std::error_code ec = turner::protocol_errc::stale_nonce;
		CHECK(ec.message() == "Stale Nonce");
	}

	SECTION("unknown")
	{
		const std::error_code ec = static_cast<turner::protocol_errc>(499);
		CHECK(ec.value() == 499);
		CHECK(ec.message() == "unknown");
		CHECK(ec.category() == turner::protocol_error_category());
		CHECK(ec.category().name() == std::string{"turner::protocol"});
	}

	SECTION("out of range")
	{
		const std::error_code ec{65536 + 400, turner::protocol_error_category()};
		CHECK(ec.message() == "unknown");
		CHECK(ec != turner::protocol_errc::bad_request);
	}

	SECTION("category")
	{
		CHECK(turner::protocol_error_category() != turner::error_category());
		const std::error_code codec = turner::errc::incomplete;
		const std::error_code protocol = static_cast<turner::protocol_errc>(codec.value());
		CHECK(codec != protocol);
	}

#undef __turner_protocol_errc_value
}

} // namespace
