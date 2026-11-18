#include <turner/codec/test.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <pal/result.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <array>
#include <concepts>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{

using turner_test::as_bytes;
using turner_test::failed;
using turner_test::parse_as;

namespace integrity = turner_test::integrity_message;

static_assert(std::same_as<turner::stun::integrity_key, turner::integrity_key<turner::stun>>);
static_assert(std::same_as<turner::turn::integrity_key, turner::stun::integrity_key>);
static_assert(std::same_as<turner::msturn::integrity_key, turner::integrity_key<turner::msturn>>);
static_assert(std::same_as<turner::stun::integrity_key::protocol_type, turner::stun>);
static_assert(std::same_as<turner::msturn::integrity_key::protocol_type, turner::msturn>);

static_assert(!std::is_default_constructible_v<turner::stun::integrity_key>);
static_assert(!std::is_copy_constructible_v<turner::stun::integrity_key>);
static_assert(!std::is_copy_assignable_v<turner::stun::integrity_key>);
static_assert(std::is_nothrow_move_constructible_v<turner::stun::integrity_key>);
static_assert(std::is_nothrow_move_assignable_v<turner::stun::integrity_key>);

static_assert(noexcept(turner::stun::integrity_key::sha1(integrity::password)));
static_assert(noexcept(turner::stun::integrity_key::sha256(integrity::password)));
static_assert(noexcept(turner::stun::sha1_key(std::declval<const turner::integrity_credentials &>())));
static_assert(noexcept(turner::stun::sha256_key(std::declval<const turner::integrity_credentials &>())));
static_assert(noexcept(turner::msturn::sha1_key(std::declval<const turner::integrity_credentials &>())));
static_assert(noexcept(turner::msturn::sha256_key(std::declval<const turner::integrity_credentials &>())));

template <typename Credentials>
concept nonce_can_be_omitted = requires
{
	Credentials{.username = "user", .realm = "realm", .password = "pass"};
};

template <typename Credentials>
concept all_fields_given = requires
{
	Credentials{.username = "user", .realm = "realm", .nonce = "", .password = "pass"};
};

static_assert(std::is_aggregate_v<turner::integrity_credentials>);
static_assert(!std::is_default_constructible_v<turner::integrity_credentials>);
static_assert(all_fields_given<turner::integrity_credentials>);
static_assert(!nonce_can_be_omitted<turner::integrity_credentials>);

constexpr auto md5_key_bytes = turner_test::bytes("8493fbc53ba582fb4c044c456bdc40eb");

struct stun
{
	using protocol_type = turner::stun;
	static constexpr auto message = turner::stun::binding.request;
	static constexpr auto &sha1_message = integrity::stun_sha1;
	static constexpr auto &sha256_message = integrity::stun_sha256;
	static constexpr auto sha256_key_bytes = turner_test::bytes(
		"07e93411" "7abd4083" "6e7c6329" "b54731b2" "b2d2a5f9" "a71f5449" "22d75e07" "30d8251b"
	);
};

struct msturn
{
	using protocol_type = turner::msturn;
	static constexpr auto message = turner::msturn::allocate;
	static constexpr auto &sha1_message = integrity::msturn_sha1;
	static constexpr auto &sha256_message = integrity::msturn_sha256;
	static constexpr auto sha256_key_bytes = turner_test::bytes(
		"5e972cd5" "7d52b576" "54d0fa6f" "6b297f2f" "073af0f8" "db0d2be1" "bc1f1b6d" "eefc5b44"
	);
};

template <typename Protocol, auto Entity>
void check_verifies (std::span<const std::byte> message, typename Protocol::integrity_key &key)
{
	std::vector<std::byte> buffer;
	const auto request = parse_as<Protocol, Entity>(message, buffer);
	CHECK(request.verify(key).has_value());
}

TEMPLATE_TEST_CASE("integrity_key", "", stun, msturn)
{
	using Protocol = typename TestType::protocol_type;
	using key_type = typename Protocol::integrity_key;

	const auto check_sha1 = [](key_type &key)
	{
		check_verifies<Protocol, TestType::message>(TestType::sha1_message, key);
	};
	const auto check_sha256 = [](key_type &key)
	{
		check_verifies<Protocol, TestType::message>(TestType::sha256_message, key);
	};

	SECTION("derived from credentials")
	{
		auto sha1 = turner_test::sha1_key<Protocol>();
		check_sha1(sha1);

		auto sha256 = turner_test::sha256_key<Protocol>();
		check_sha256(sha256);
	}

	SECTION("key from bytes")
	{
		auto sha1 = key_type::sha1(md5_key_bytes);
		REQUIRE(sha1);
		check_sha1(*sha1);

		auto sha256 = key_type::sha256(TestType::sha256_key_bytes);
		REQUIRE(sha256);
		check_sha256(*sha256);
	}

	SECTION("password as bytes")
	{
		const auto password_bytes = as_bytes(integrity::password);
		const std::vector<std::byte> password{password_bytes.begin(), password_bytes.end()};
		const turner::integrity_credentials credentials
		{
			.username = integrity::username,
			.realm = integrity::realm,
			.nonce = integrity::nonce,
			.password = password,
		};

		auto sha1 = Protocol::sha1_key(credentials);
		REQUIRE(sha1);
		check_sha1(*sha1);

		auto sha256 = Protocol::sha256_key(credentials);
		REQUIRE(sha256);
		check_sha256(*sha256);
	}

	SECTION("fields as text")
	{
		const std::string username{integrity::username};
		auto from_string = Protocol::sha1_key({
			.username = username,
			.realm = integrity::realm,
			.nonce = integrity::nonce,
			.password = std::string{integrity::password},
		});
		REQUIRE(from_string);
		check_sha1(*from_string);

		auto from_literal = Protocol::sha1_key({
			.username = "user",
			.realm = "realm",
			.nonce = "nonce",
			.password = "pass",
		});
		REQUIRE(from_literal);
		check_sha1(*from_literal);
	}

	SECTION("different credentials")
	{
		const auto [username, realm, password] = GENERATE(
			std::array<std::string_view, 3>{"other", "realm", "pass"},
			std::array<std::string_view, 3>{"user", "other", "pass"},
			std::array<std::string_view, 3>{"user", "realm", "other"}
		);
		const turner::integrity_credentials credentials
		{
			.username = username,
			.realm = realm,
			.nonce = integrity::nonce,
			.password = password,
		};

		std::vector<std::byte> sha1_buffer;
		const auto sha1_request = parse_as<Protocol, TestType::message>(TestType::sha1_message, sha1_buffer);
		auto sha1 = Protocol::sha1_key(credentials);
		REQUIRE(sha1);
		CHECK(sha1_request.verify(*sha1) == failed(turner::errc::integrity_mismatch));

		std::vector<std::byte> sha256_buffer;
		const auto sha256_request = parse_as<Protocol, TestType::message>(
			TestType::sha256_message,
			sha256_buffer
		);
		auto sha256 = Protocol::sha256_key(credentials);
		REQUIRE(sha256);
		CHECK(sha256_request.verify(*sha256) == failed(turner::errc::integrity_mismatch));
	}

	SECTION("moved key")
	{
		auto source = turner_test::sha1_key<Protocol>();
		auto key = std::move(source);
		check_sha1(key);

		auto assigned = turner_test::sha256_key<Protocol>();
		assigned = std::move(key);
		check_sha1(assigned);
	}
}

TEST_CASE("integrity_key/protocol specific")
{
	constexpr auto binding = turner::stun::binding.request;
	constexpr auto allocate = turner::msturn::allocate;

	SECTION("stun: rfc5769 short term credentials")
	{
		constexpr std::string_view password = "VOkJxbRl1RmTxUk/WvJxBt";
		auto key = turner::stun::integrity_key::sha1(password);
		REQUIRE(key);

		check_verifies<turner::stun, binding>(turner_test::rfc5769::request, *key);
		check_verifies<turner::stun, turner::stun::binding.success>(turner_test::rfc5769::ipv4_response, *key);
		check_verifies<turner::stun, turner::stun::binding.success>(turner_test::rfc5769::ipv6_response, *key);
	}

	SECTION("stun: rfc5769 long term credentials")
	{
		auto key = turner::stun::sha1_key({
			.username = "\xe3\x83\x9e\xe3\x83\x88\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9",
			.realm = "example.org",
			.nonce = "",
			.password = "TheMatrIX",
		});
		REQUIRE(key);
		check_verifies<turner::stun, binding>(turner_test::rfc5769::long_term_request, *key);
	}

	SECTION("stun: empty realm")
	{
		auto key = turner::stun::sha1_key({
			.username = integrity::username,
			.realm = "",
			.nonce = "",
			.password = integrity::password,
		});
		REQUIRE(key);
		check_verifies<turner::stun, binding>(integrity::stun_empty_realm, *key);
	}

	SECTION("stun: md5 key with sha256")
	{
		auto key = turner::stun::integrity_key::sha256(md5_key_bytes);
		REQUIRE(key);
		check_verifies<turner::stun, binding>(integrity::stun_md5_key_sha256, *key);
	}

	SECTION("turn: stun key")
	{
		auto key = turner_test::sha1_key<turner::stun>();
		check_verifies<turner::turn, turner::turn::allocate>(integrity::turn_allocate_sha1, key);
	}

	SECTION("nonce is ignored unless used")
	{
		const auto nonce = GENERATE(std::string_view{}, std::string_view{"other"});
		const turner::integrity_credentials credentials
		{
			.username = integrity::username,
			.realm = integrity::realm,
			.nonce = nonce,
			.password = integrity::password,
		};

		auto stun_sha1 = turner::stun::sha1_key(credentials);
		REQUIRE(stun_sha1);
		check_verifies<turner::stun, binding>(integrity::stun_sha1, *stun_sha1);

		auto stun_sha256 = turner::stun::sha256_key(credentials);
		REQUIRE(stun_sha256);
		check_verifies<turner::stun, binding>(integrity::stun_sha256, *stun_sha256);

		auto msturn_sha1 = turner::msturn::sha1_key(credentials);
		REQUIRE(msturn_sha1);
		check_verifies<turner::msturn, allocate>(integrity::msturn_sha1, *msturn_sha1);
	}

	SECTION("msturn: realm attribute keeps quotes, key uses realm without them")
	{
		const auto &message = GENERATE(
			std::span<const std::byte>{integrity::msturn_sha1},
			std::span<const std::byte>{integrity::msturn_sha256}
		);
		std::vector<std::byte> buffer;
		const auto request = parse_as<turner::msturn, allocate>(message, buffer);
		CHECK(request.read(turner::msturn::realm) == integrity::quoted_realm);

		auto sha1 = turner_test::sha1_key<turner::msturn>();
		auto sha256 = turner_test::sha256_key<turner::msturn>();
		auto &key = request.integrity() == turner::integrity_algorithm::sha1 ? sha1 : sha256;
		CHECK(request.verify(key).has_value());
	}

	SECTION("msturn: sha256_key with nonce longer than hmac block")
	{
		const std::string nonce(65, 'n');
		auto key = turner::msturn::sha256_key({
			.username = integrity::username,
			.realm = integrity::realm,
			.nonce = nonce,
			.password = integrity::password,
		});
		REQUIRE(key);
		check_verifies<turner::msturn, allocate>(integrity::msturn_long_nonce, *key);
	}

	SECTION("msturn: sha256_key depends on nonce")
	{
		std::vector<std::byte> buffer;
		const auto request = parse_as<turner::msturn, allocate>(integrity::msturn_sha256, buffer);
		auto key = turner::msturn::sha256_key({
			.username = integrity::username,
			.realm = integrity::realm,
			.nonce = "other",
			.password = integrity::password,
		});
		REQUIRE(key);
		CHECK(request.verify(*key) == failed(turner::errc::integrity_mismatch));
	}
}

} // namespace
