#include <turner/attribute_type>
#include <turner/test>

namespace {

using namespace turner_test;

struct Protocol
{
	struct attribute_value_type { };
	static constexpr auto a1 = turner::attribute<Protocol, attribute_value_type, 0x0001>;
	static constexpr auto a2 = turner::attribute<Protocol, attribute_value_type, 0x8001>;
};

TEST_CASE("attribute_type")
{
	static_assert(std::is_same_v<typename decltype(Protocol::a1)::protocol_type, Protocol>);
	static_assert(Protocol::a1.type == 0x0001);
	static_assert(Protocol::a1.is_comprehension_required);
	static_assert(!Protocol::a1.is_comprehension_optional);

	static_assert(std::is_same_v<typename decltype(Protocol::a2)::protocol_type, Protocol>);
	static_assert(Protocol::a2.type == 0x8001);
	static_assert(!Protocol::a2.is_comprehension_required);
	static_assert(Protocol::a2.is_comprehension_optional);
}

} // namespace
