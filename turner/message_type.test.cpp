#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using message_type = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(message_type, protocol_types);


TYPED_TEST(message_type, type)
{
  EXPECT_EQ(
    TypeParam::msg_type_v(),
    TypeParam::msg_type().type()
  );
  EXPECT_TRUE(TypeParam::msg_type().is_request());
  EXPECT_FALSE(TypeParam::msg_type().is_success_response());
  EXPECT_FALSE(TypeParam::msg_type().is_error_response());
  EXPECT_FALSE(TypeParam::msg_type().is_indication());
}


TYPED_TEST(message_type, compare)
{
  auto t = TypeParam::msg_type();

  EXPECT_EQ(TypeParam::msg_type(), t);
  EXPECT_EQ(t, TypeParam::msg_type());

  EXPECT_NE(t, t.success_response());
  EXPECT_NE(t.success_response(), t);
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


TYPED_TEST(message_type, success_response)
{
  auto t = TypeParam::msg_type();
  auto ts = t.success_response();

  EXPECT_NE(t, ts);
  EXPECT_NE(ts, t);

  EXPECT_FALSE(ts.is_request());
  EXPECT_TRUE(ts.is_success_response());
  EXPECT_FALSE(ts.is_error_response());
  EXPECT_FALSE(ts.is_indication());

  EXPECT_EQ(
    typeid(typename decltype(t)::success_response_t),
    typeid(ts)
  );
}


TYPED_TEST(message_type, error_response)
{
  auto t = TypeParam::msg_type();
  auto ts = t.error_response();

  EXPECT_NE(t, ts);
  EXPECT_NE(ts, t);

  EXPECT_FALSE(ts.is_request());
  EXPECT_FALSE(ts.is_success_response());
  EXPECT_TRUE(ts.is_error_response());
  EXPECT_FALSE(ts.is_indication());

  EXPECT_EQ(
    typeid(typename decltype(t)::error_response_t),
    typeid(ts)
  );
}


TYPED_TEST(message_type, indication)
{
  auto t = TypeParam::msg_type();
  auto ts = t.indication();

  EXPECT_NE(t, ts);
  EXPECT_NE(ts, t);

  EXPECT_FALSE(ts.is_request());
  EXPECT_FALSE(ts.is_success_response());
  EXPECT_FALSE(ts.is_error_response());
  EXPECT_TRUE(ts.is_indication());

  EXPECT_EQ(
    typeid(typename decltype(t)::indication_t),
    typeid(ts)
  );
}


}} // namespace turner_test
