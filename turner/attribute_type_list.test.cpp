#include <turner/attribute_type_list>
#include <turner/test>

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

} // namespace
