#include <turner/message_type>
#include <turner/test>
#include <catch2/catch_test_macros.hpp>


namespace {


struct P
{
	static constexpr turner::message_type<P, 1> m1{};
	static constexpr turner::message_type<P, 2> m2{};
};


TEST_CASE("message_type")
{
	CHECK(P::m1 == P::m1);
	CHECK(P::m1 != P::m2);

	CHECK(P::m1 == 1);
	CHECK(P::m1 != 2);

	CHECK(1 == P::m1);
	CHECK(2 != P::m1);

	CHECK(P::m2 != 1);
	CHECK(P::m2 == 2);

	CHECK(1 != P::m2);
	CHECK(2 == P::m2);
}


} // namespace
