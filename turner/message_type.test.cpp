#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using message_type = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(message_type, protocol_types);


TYPED_TEST(message_type, type)
{
  EXPECT_EQ(
    msg_type_v(TypeParam()),
    msg_type(TypeParam()).type()
  );
  EXPECT_TRUE(msg_type(TypeParam()).is_request());
  EXPECT_FALSE(msg_type(TypeParam()).is_success_response());
  EXPECT_FALSE(msg_type(TypeParam()).is_error_response());
  EXPECT_FALSE(msg_type(TypeParam()).is_indication());
}


TYPED_TEST(message_type, name)
{
  auto t = msg_type(TypeParam());

  const char *expected;
  t >> expected;
  EXPECT_STREQ(expected, t.name());
}


TYPED_TEST(message_type, name_unnamed)
{
  EXPECT_EQ(nullptr, unused_message_type<TypeParam>.name());
}


TYPED_TEST(message_type, ostream)
{
  auto t = msg_type(TypeParam());

  const char *expected;
  t >> expected;

  std::ostringstream oss;
  oss << t;
  EXPECT_EQ(expected, oss.str());
}


TYPED_TEST(message_type, ostream_unnamed_with_named_protocol)
{
  std::ostringstream oss, expected;

  expected
    << TypeParam::name()
    << ':'
    << unused_message_type<TypeParam>.type();

  oss << unused_message_type<TypeParam>;
  EXPECT_EQ(expected.str(), oss.str());
}


TEST(message_type, ostream_unnamed_with_unnamed_protocol)
{
  std::ostringstream oss;
  oss << unused_message_type<unnamed_protocol_t>;

  EXPECT_EQ(
    std::to_string(unused_message_type<unnamed_protocol_t>.type()),
    oss.str()
  );
}


TYPED_TEST(message_type, compare)
{
  auto t = msg_type(TypeParam());

  EXPECT_EQ(msg_type(TypeParam()), t);
  EXPECT_EQ(t, msg_type(TypeParam()));

  EXPECT_NE(t, t.success_response());
  EXPECT_NE(t.success_response(), t);
}


TYPED_TEST(message_type, compare_value)
{
  auto t = msg_type(TypeParam());
  auto v = msg_type_v(TypeParam());

  EXPECT_EQ(v, t);
  EXPECT_EQ(t, v);

  EXPECT_NE(uint16_t(v + 1), t);
  EXPECT_NE(t, uint16_t(v + 1));
}


TYPED_TEST(message_type, success_response)
{
  auto t = msg_type(TypeParam());
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
  auto t = msg_type(TypeParam());
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
  auto t = msg_type(TypeParam());
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
