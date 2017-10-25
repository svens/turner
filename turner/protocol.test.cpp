#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using protocol = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(protocol, protocol_types);


TYPED_TEST(protocol, message_type)
{
  auto t = TypeParam::template message_type<msg_type_v(TypeParam())>();
  EXPECT_EQ(typeid(msg_type(TypeParam())), typeid(t));
  EXPECT_NE(typeid(t), typeid(unused_message_type<TypeParam>));
}


TYPED_TEST(protocol, name)
{
  const char *expected;
  TypeParam() >> expected;
  EXPECT_STREQ(expected, TypeParam::name());
}


TEST(protocol, name_unnamed)
{
  EXPECT_EQ(nullptr, unnamed_protocol.name());
}


TYPED_TEST(protocol, ostream)
{
  const char *expected;
  TypeParam() >> expected;

  std::ostringstream oss;
  oss << TypeParam();
  EXPECT_EQ(expected, oss.str());
}


TEST(protocol, ostream_unnamed)
{
  std::ostringstream oss;
  oss << unnamed_protocol;
  EXPECT_EQ("", oss.str());
}


TYPED_TEST(protocol, from_wire)
{
  auto data = msg_data(TypeParam());

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(msg_type(TypeParam()), msg->type());
}


}} // namespace turner_test
