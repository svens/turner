#include <turner/attribute_type>
#include <turner/test>


namespace {

using namespace turner_test;


struct P
{
	struct value_1_processor { };
	struct value_2_processor { };

	static constexpr turner::attribute_type<P, value_1_processor> a1 = 0x0001;
	static constexpr turner::attribute_type<P, value_2_processor> a2 = 0x8001;
};


TEST_CASE("attribute_type")
{
	CHECK(P::a1 == P::a1);
	CHECK(P::a1 != P::a2);
	CHECK(P::a1.is_comprehension_required());
	CHECK_FALSE(P::a1.is_comprehension_optional());

	CHECK(P::a2 == P::a2);
	CHECK(P::a2 != P::a1);
	CHECK_FALSE(P::a2.is_comprehension_required());
	CHECK(P::a2.is_comprehension_optional());

	CHECK(P::a1 == 0x0001_u16);
	CHECK(P::a1 != 0x8001_u16);

	CHECK(0x0001_u16 == P::a1);
	CHECK(0x8001_u16 != P::a1);

	CHECK(P::a2 != 0x0001_u16);
	CHECK(P::a2 == 0x8001_u16);

	CHECK(0x0001_u16 != P::a2);
	CHECK(0x8001_u16 == P::a2);
}


} // namespace
