#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using message_type = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(message_type, protocol_types);


TYPED_TEST(message_type, protocol)
{
  EXPECT_EQ(
    typeid(typename decltype(msg_type(TypeParam()))::protocol_t),
    typeid(TypeParam)
  );
}


TYPED_TEST(message_type, type)
{
  EXPECT_EQ(
    msg_type_v(TypeParam()),
    msg_type(TypeParam()).type()
  );
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

  EXPECT_EQ(
    t.type() | turner::__bits::success_response_class,
    ts.type()
  );

  EXPECT_EQ(
    typeid(typename decltype(t)::success_response_t),
    typeid(ts)
  );

  EXPECT_EQ(
    typeid(typename decltype(ts)::protocol_t),
    typeid(TypeParam)
  );
}


TYPED_TEST(message_type, error_response)
{
  auto t = msg_type(TypeParam());
  auto ts = t.error_response();

  EXPECT_NE(t, ts);
  EXPECT_NE(ts, t);

  EXPECT_EQ(
    t.type() | turner::__bits::error_response_class,
    ts.type()
  );

  EXPECT_EQ(
    typeid(typename decltype(t)::error_response_t),
    typeid(ts)
  );

  EXPECT_EQ(
    typeid(typename decltype(ts)::protocol_t),
    typeid(TypeParam)
  );
}


TYPED_TEST(message_type, indication)
{
  auto t = msg_type(TypeParam());
  auto ts = t.indication();

  EXPECT_NE(t, ts);
  EXPECT_NE(ts, t);

  EXPECT_EQ(
    t.type() | turner::__bits::indication_class,
    ts.type()
  );

  EXPECT_EQ(
    typeid(typename decltype(t)::indication_t),
    typeid(ts)
  );

  EXPECT_EQ(
    typeid(typename decltype(ts)::protocol_t),
    typeid(TypeParam)
  );
}


}} // namespace turner_test
