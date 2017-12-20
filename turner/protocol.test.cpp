#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using protocol = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(protocol, protocol_types);


TYPED_TEST(protocol, parse)
{
  auto data = TypeParam::msg_data();

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(TypeParam::msg_type(), msg->type());
}


TYPED_TEST(protocol, build_range)
{
  std::array<uint8_t, TypeParam::header_and_cookie_size() + 4> data;
  data.fill(0);

  std::error_code error;
  auto writer = TypeParam::build(TypeParam::msg_type(),
    data.begin(), data.end(),
    error
  );
  ASSERT_TRUE(!error);
  EXPECT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_type(), writer.type());
  EXPECT_EQ(4U, writer.available());

  auto [ begin, end ] = writer.finish();
  EXPECT_EQ(&data[0], begin);
  EXPECT_EQ(&data[TypeParam::header_and_cookie_size()], end);

  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(TypeParam::msg_type(), msg->type());
  EXPECT_EQ(0U + TypeParam::min_payload_length(), msg->length());
  EXPECT_EQ(TypeParam::traits_t::cookie, msg->cookie());

  std::array<uint8_t, TypeParam::traits_t::transaction_id_size> null_transaction_id;
  null_transaction_id.fill(0);
  EXPECT_NE(null_transaction_id, msg->transaction_id());
}


TYPED_TEST(protocol, build_range_not_enough_room_header)
{
  std::array<uint8_t, TypeParam::header_and_cookie_size() - 1> data;
  data.fill(0);

  auto original = data;

  std::error_code error;
  auto writer = TypeParam::build(TypeParam::msg_type(),
    data.begin(), data.end(),
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);
  EXPECT_EQ(original, data);
}


TYPED_TEST(protocol, build_range_not_enough_room_for_finish)
{
  std::array<uint8_t, TypeParam::header_and_cookie_size() + 1> data;
  data.fill(0);

  std::error_code error;
  auto writer = TypeParam::build(TypeParam::msg_type(),
    data.begin(), data.end(),
    error
  );
  ASSERT_TRUE(!error);
  EXPECT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_type(), writer.type());
  EXPECT_EQ(1U, writer.available());

  auto integrity_calculator = TypeParam::msg_hmac();
  (void)writer.finish(integrity_calculator, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
}


TYPED_TEST(protocol, build_data)
{
  std::array<uint8_t, TypeParam::header_and_cookie_size() + 4> data;
  data.fill(0);

  std::error_code error;
  auto writer = TypeParam::build(TypeParam::msg_type(), data, error);
  ASSERT_TRUE(!error);
  EXPECT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_type(), writer.type());
  EXPECT_EQ(4U, writer.available());

  auto [ begin, end ] = writer.finish();
  EXPECT_EQ(&data[0], begin);
  EXPECT_EQ(&data[TypeParam::header_and_cookie_size()], end);

  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(TypeParam::msg_type(), msg->type());
  EXPECT_EQ(0U + TypeParam::min_payload_length(), msg->length());
  EXPECT_EQ(TypeParam::traits_t::cookie, msg->cookie());

  std::array<uint8_t, TypeParam::traits_t::transaction_id_size> null_transaction_id;
  null_transaction_id.fill(0);
  EXPECT_NE(null_transaction_id, msg->transaction_id());
}


TYPED_TEST(protocol, build_data_not_enough_room_header)
{
  std::array<uint8_t, TypeParam::header_and_cookie_size() - 1> data;
  data.fill(0);

  auto original = data;

  std::error_code error;
  auto writer = TypeParam::build(TypeParam::msg_type(), data, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);
  EXPECT_EQ(original, data);
}


TYPED_TEST(protocol, build_data_not_enough_room_for_finish)
{
  std::array<uint8_t, TypeParam::header_and_cookie_size() + 1> data;
  data.fill(0);

  std::error_code error;
  auto writer = TypeParam::build(TypeParam::msg_type(), data, error);
  ASSERT_TRUE(!error);
  EXPECT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_type(), writer.type());
  EXPECT_EQ(1U, writer.available());

  auto integrity_calculator = TypeParam::msg_hmac();
  (void)writer.finish(integrity_calculator, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
}


}} // namespace turner_test
