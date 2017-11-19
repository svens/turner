#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using protocol = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(protocol, protocol_types);


TYPED_TEST(protocol, name)
{
  EXPECT_STREQ(TypeParam::expected_name, TypeParam::name());
}


TEST(protocol, name_unnamed)
{
  EXPECT_EQ(nullptr, unnamed_protocol.name());
}


TYPED_TEST(protocol, ostream)
{
  std::ostringstream oss;
  oss << TypeParam();
  EXPECT_EQ(TypeParam::expected_name, oss.str());
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
  std::array<uint8_t, TypeParam::traits_t::header_size + 4> data;
  data.fill(0);

  std::error_code error;
  auto writer = TypeParam::build(msg_type(TypeParam()),
    data.begin(), data.end(),
    error
  );
  EXPECT_TRUE(!error);
  EXPECT_FALSE(!writer);
  EXPECT_EQ(msg_type(TypeParam()), writer.type());
  EXPECT_EQ(4U, writer.available());

  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(msg_type(TypeParam()), msg->type());
  EXPECT_EQ(0U, msg->length());
  EXPECT_EQ(TypeParam::traits_t::cookie, msg->cookie());

  std::array<uint8_t, TypeParam::traits_t::transaction_id_size> null_transaction_id;
  null_transaction_id.fill(0);
  EXPECT_NE(null_transaction_id, msg->transaction_id());
}


TYPED_TEST(protocol, build_not_enough_room)
{
  std::array<uint8_t, TypeParam::traits_t::header_size - 1> data;
  data.fill(0);

  auto original = data;

  std::error_code error;
  auto writer = TypeParam::build(msg_type(TypeParam()),
    data.begin(), data.end(),
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);
  EXPECT_EQ(original, data);
}


}} // namespace turner_test
