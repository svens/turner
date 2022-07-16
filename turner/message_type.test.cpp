#include <turner/message_type>
#include <turner/test>


namespace {


struct P
{
	static constexpr auto m1 = turner::message_type<P>::request<1>();
	static constexpr auto m2 = turner::message_type<P>::request<2>();
};


TEST_CASE("message_type")
{
	CHECK(P::m1 == P::m1);
	CHECK(P::m1 != P::m2);

	CHECK(P::m1 == uint16_t{1});
	CHECK(P::m1 != uint16_t{2});

	CHECK(uint16_t{1} == P::m1);
	CHECK(uint16_t{2} != P::m1);

	CHECK(P::m2 != uint16_t{1});
	CHECK(P::m2 == uint16_t{2});

	CHECK(uint16_t{1} != P::m2);
	CHECK(uint16_t{2} == P::m2);
}


} // namespace
