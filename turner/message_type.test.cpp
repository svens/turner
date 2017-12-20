#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using message_type = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(message_type, protocol_types);


TYPED_TEST(message_type, type)
{
  EXPECT_EQ(TypeParam::msg_type_v(), TypeParam::msg_type().type);
}


TYPED_TEST(message_type, compare)
{
  auto t = TypeParam::msg_type();
  EXPECT_EQ(TypeParam::msg_type(), t);
  EXPECT_EQ(t, TypeParam::msg_type());
}


TYPED_TEST(message_type, compare_value)
{
  auto t = TypeParam::msg_type();
  auto v = TypeParam::msg_type_v();

  EXPECT_EQ(v, t);
  EXPECT_EQ(t, v);

  EXPECT_NE(uint16_t(v + 1), t);
  EXPECT_NE(t, uint16_t(v + 1));
}


}} // namespace turner_test
