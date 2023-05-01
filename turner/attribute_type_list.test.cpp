#include <turner/attribute_type_list>
#include <turner/msturn>
#include <turner/stun>
#include <turner/turn>
#include <turner/test>
#include <catch2/catch_template_test_macros.hpp>

namespace {

using namespace turner_test;

TEST_CASE("attribute_type_list")
{
	using Type = turner::attribute_type_list<
		test_protocol::a0,
		test_protocol::a1,
		test_protocol::a2
	>;
	__turner_check(std::is_same_v<Type::protocol_type, test_protocol>);

	auto var = turner::attributes<
		test_protocol::a0,
		test_protocol::a1,
		test_protocol::a2
	>;
	__turner_check(std::is_same_v<decltype(var), Type>);

	SECTION("type")
	{
		__turner_check(std::tuple_size_v<Type> == 3);
		__turner_check(std::is_same_v<std::tuple_element_t<0, Type>, std::decay_t<decltype(test_protocol::a0)>>);
		__turner_check(std::is_same_v<std::tuple_element_t<1, Type>, std::decay_t<decltype(test_protocol::a1)>>);
		__turner_check(std::is_same_v<std::tuple_element_t<2, Type>, std::decay_t<decltype(test_protocol::a2)>>);
	}

	SECTION("var")
	{
		__turner_check(std::get<0>(var) == test_protocol::a0.type);
		__turner_check(std::get<1>(var) == test_protocol::a1.type);
		__turner_check(std::get<2>(var) == test_protocol::a2.type);

		__turner_check(var.contains(test_protocol::a0.type));
		__turner_check(var.contains(test_protocol::a1.type));
		__turner_check(var.contains(test_protocol::a2.type));
		__turner_check(!var.contains(0xffff));
	}

	SECTION("comprehension_required")
	{
		auto required = var.comprehension_required();
		__turner_check(!std::is_same_v<decltype(required), decltype(var)>);

		__turner_check(std::tuple_size_v<decltype(required)> == 2);
		__turner_check(std::get<0>(required) == test_protocol::a0.type);
		__turner_check(std::get<1>(required) == test_protocol::a1.type);

		__turner_check(required.contains(test_protocol::a0.type));
		__turner_check(required.contains(test_protocol::a1.type));
		__turner_check(!required.contains(test_protocol::a2.type));
		__turner_check(!required.contains(0xffff));

		auto required_1 = required.comprehension_required();
		__turner_check(!std::is_same_v<decltype(required_1), decltype(var)>);
		__turner_check(std::is_same_v<decltype(required_1), decltype(required)>);
	}

	#if __turner_build_error_not_distinct_types_v
	{
		// define symbol above to check for static_assert errors
		std::ignore = turner::attributes<
			test_protocol::a0,
			test_protocol::a0,
			test_protocol::a1
		>;
		std::ignore = turner::attributes<
			test_protocol::a0,
			test_protocol::a1,
			test_protocol::a0
		>;
		std::ignore = turner::attributes<
			test_protocol::a1,
			test_protocol::a0,
			test_protocol::a0
		>;
	}
	#endif
}

struct msturn //{{{1
{
	using protocol_type = turner::msturn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x24, // MS-TURN Allocation
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
		0x80, 0x00, 0x00, 0x04, // Custom
		't',  'e',  's',  't',
	};
};

struct stun //{{{1
{
	using protocol_type = turner::stun;

	static constexpr uint8_t message[] =
	{
		0x00, 0x01, 0x00, 0x1c, // STUN Binding
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x00, 0x14, 0x00, 0x05, // Realm
		'r',  'e',  'a',  'l',
		'm',  0x00, 0x00, 0x00,
		0x00, 0x06, 0x00, 0x04, // Username
		'u',  's',  'e',  'r',
		0x80, 0x00, 0x00, 0x04, // Custom
		't',  'e',  's',  't',
	};
};

struct turn //{{{1
{
	using protocol_type = turner::turn;

	static constexpr uint8_t message[] =
	{
		0x00, 0x03, 0x00, 0x1c, // TURN Allocate
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x00, 0x14, 0x00, 0x05, // Realm
		'r',  'e',  'a',  'l',
		'm',  0x00, 0x00, 0x00,
		0x00, 0x06, 0x00, 0x04, // Username
		'u',  's',  'e',  'r',
		0x80, 0x00, 0x00, 0x04, // Custom
		't',  'e',  's',  't',
	};
};

// }}}1

TEMPLATE_TEST_CASE("attribute_type_list", "",
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
		auto [realm, username] = reader->read(attributes);
		CHECK(realm.value() == "realm");
		CHECK(username.value() == "user");
	}

	SECTION("read: missing")
	{
		auto attributes = turner::attributes<Protocol::realm, Protocol::nonce>;
		auto [realm, nonce] = reader->read(attributes);
		CHECK(realm.value() == "realm");
		CHECK(nonce.error_or(std::error_code{}) == turner::errc::attribute_not_found);
	}

	SECTION("not_read")
	{
		static constexpr auto custom_attribute = turner::attribute<
			Protocol,
			turner::string_value_type<>,
			0x8000
		>;

		auto attributes = turner::attributes<Protocol::realm, custom_attribute>;
		std::array<uint16_t, 2> list;
		auto count = reader->not_read(std::span{list}, attributes);
		REQUIRE(count == 1);
		CHECK(list[0] == Protocol::username);
	}

	SECTION("not_read: overflow")
	{
		auto attributes = turner::attributes<Protocol::realm>;
		std::array<uint16_t, 2> list{};
		auto count = reader->not_read(std::span{list}.first(0), attributes);
		REQUIRE(count == 1);
		CHECK(list[0] == 0);
	}
}

} // namespace
