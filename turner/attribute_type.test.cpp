#include <turner/attribute_type>
#include <turner/test>

namespace {

using namespace turner_test;

TEST_CASE("attribute_type")
{
	__turner_check(std::is_same_v<typename decltype(test_protocol::a0)::protocol_type, test_protocol>);
	__turner_check(test_protocol::a0 == 0x0001);
	__turner_check(test_protocol::a0 != 0x0002);
	__turner_check(test_protocol::a0.is_comprehension_required);
	__turner_check(!test_protocol::a0.is_comprehension_optional);

	__turner_check(std::is_same_v<typename decltype(test_protocol::a1)::protocol_type, test_protocol>);
	__turner_check(test_protocol::a1 == 0x0002);
	__turner_check(test_protocol::a1 != 0x0001);
	__turner_check(test_protocol::a1.is_comprehension_required);
	__turner_check(!test_protocol::a1.is_comprehension_optional);

	__turner_check(std::is_same_v<typename decltype(test_protocol::a2)::protocol_type, test_protocol>);
	__turner_check(test_protocol::a2 == 0x8001);
	__turner_check(test_protocol::a2 != 0x8002);
	__turner_check(!test_protocol::a2.is_comprehension_required);
	__turner_check(test_protocol::a2.is_comprehension_optional);
}

} // namespace
