#include <turner/message_type>
#include <turner/test>


namespace {

using namespace turner_test;


struct P
{
	static constexpr turner::message_type<P> m1 = 1;
	static constexpr turner::message_type<P> m2 = 2;
};


TEST_CASE("message_type")
{
	CHECK(P::m1 == P::m1);
	CHECK(P::m1 != P::m2);

	CHECK(P::m1 == 1_u16);
	CHECK(P::m1 != 2_u16);

	CHECK(1_u16 == P::m1);
	CHECK(2_u16 != P::m1);

	CHECK(P::m2 != 1_u16);
	CHECK(P::m2 == 2_u16);

	CHECK(1_u16 != P::m2);
	CHECK(2_u16 == P::m2);

	CHECK_THROWS(turner::message_type<P>{0xff01});
}


} // namespace
