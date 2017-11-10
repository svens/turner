#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using protocol = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(protocol, protocol_types);


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


TYPED_TEST(protocol, parse)
{
  auto data = msg_data(TypeParam());

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(msg_type(TypeParam()), msg->type());
}


TYPED_TEST(protocol, build)
{
  std::array<uint8_t, TypeParam::traits_t::header_size> data;
  data.fill(0);

  std::error_code error;
  TypeParam::build(msg_type(TypeParam()),
    data.begin(), data.end(),
    error
  );
  EXPECT_TRUE(!error);

  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(msg_type(TypeParam()), msg->type());
  EXPECT_EQ(0U, msg->length());
  EXPECT_EQ(TypeParam::traits_t::cookie, msg->cookie());

  std::array<uint8_t, TypeParam::traits_t::transaction_id_size> null_transaction_id;
  EXPECT_NE(null_transaction_id, msg->transaction_id());
}


}} // namespace turner_test
