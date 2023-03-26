#include <turner/attribute_list>
#include <turner/msturn>
#include <turner/stun>
#include <turner/turn>
#include <turner/test>
#include <catch2/catch_template_test_macros.hpp>

namespace {

using namespace turner_test;

struct msturn //{{{1
{
	using protocol_type = turner::msturn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x1c, // MS-TURN Allocation
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x0f, 0x00, 0x04, // Magic Cookie
		0x72, 0xc6, 0x4b, 0xc6,
		0x00, 0x15, 0x00, 0x05, // Realm
		'r',  'e',  'a',  'l',
		'm',  0x00, 0x00, 0x00,
		0x00, 0x06, 0x00, 0x04, // Username
		'u',  's',  'e',  'r',
	};
};

struct stun //{{{1
{
	using protocol_type = turner::stun;

	static constexpr uint8_t message[] =
	{
		0x00, 0x01, 0x00, 0x14, // STUN Binding
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x00, 0x14, 0x00, 0x05, // Realm
		'r',  'e',  'a',  'l',
		'm',  0x00, 0x00, 0x00,
		0x00, 0x06, 0x00, 0x04, // Username
		'u',  's',  'e',  'r',
	};
};

struct turn //{{{1
{
	using protocol_type = turner::turn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x14, // TURN Allocate
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x00, 0x14, 0x00, 0x05, // Realm
		'r',  'e',  'a',  'l',
		'm',  0x00, 0x00, 0x00,
		0x00, 0x06, 0x00, 0x04, // Username
		'u',  's',  'e',  'r',
	};
};

//}}}1

TEMPLATE_TEST_CASE("attribute_list", "",
	msturn,
	stun,
	turn)
{
	using Protocol = typename TestType::protocol_type;

	auto span = std::as_bytes(std::span{TestType::message});
	auto reader = Protocol::read_message(span);
	REQUIRE(reader);

	SECTION("read")
	{
		auto attributes = turner::attributes<Protocol::realm, Protocol::username>;
		auto [realm, username] = reader->read_all(attributes);
		CHECK(realm.value() == "realm");
		CHECK(username.value() == "user");
	}

	SECTION("read: missing")
	{
		auto attributes = turner::attributes<Protocol::realm, Protocol::nonce>;
		auto [realm, nonce] = reader->read_all(attributes);
		CHECK(realm.value() == "realm");
		CHECK(nonce.error_or(std::error_code{}) == turner::errc::attribute_not_found);
	}

	SECTION("unread_comprehension_required")
	{
		auto attributes = turner::attributes<Protocol::realm>;
		std::array<uint16_t, 2> unread;
		auto count = reader->unread_comprehension_required(attributes, unread);
		REQUIRE(count == 1);
		CHECK(unread[0] == Protocol::username.type);
	}

	SECTION("unread_comprehension_required overflow")
	{
		auto attributes = turner::attributes<Protocol::realm>;
		std::array<uint16_t, 0> unread;
		auto count = reader->unread_comprehension_required(attributes, unread);
		REQUIRE(count == 1);
	}
}

} // namespace
