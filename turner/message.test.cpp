#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using any_message = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(any_message, protocol_types);


TYPED_TEST(any_message, insufficient_header_data)
{
  auto data = msg_data(TypeParam());
  data.resize(data.size() / 2);

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_header_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, type)
{
  auto data = msg_data(TypeParam());

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(msg_type_v(TypeParam()), msg->type());
  EXPECT_TRUE(msg->is_request());
  EXPECT_FALSE(msg->is_success_response());
  EXPECT_FALSE(msg->is_error_response());
  EXPECT_FALSE(msg->is_indication());
}


TYPED_TEST(any_message, invalid_type)
{
  auto data = msg_data(TypeParam());
  data[0] |= 0b1100'0000;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_type, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, length)
{
  auto data = msg_data(TypeParam());

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(msg_len(TypeParam()), msg->length());
}


TYPED_TEST(any_message, invalid_length)
{
  auto data = msg_data(TypeParam());
  data[3] += 1;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_length, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, insufficient_payload_data)
{
  auto data = msg_data(TypeParam());
  data.pop_back();

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, invalid_cookie)
{
  auto data = msg_data(TypeParam());
  data[TypeParam::traits_t::cookie_offset] += 1;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_cookie, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, transaction_id)
{
  auto data = msg_data(TypeParam());

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(txn_id(TypeParam()), msg->transaction_id());
}


TYPED_TEST(any_message, try_as_valid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(msg_type(TypeParam()));
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(msg_type(TypeParam()), msg->type());

  EXPECT_EQ(any_msg, msg);
  EXPECT_EQ(any_msg->type(), msg->type());
  EXPECT_EQ(any_msg->length(), msg->length());
  EXPECT_EQ(any_msg->cookie(), msg->cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg->transaction_id());
}


TYPED_TEST(any_message, as_valid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(msg_type(TypeParam()));
  EXPECT_EQ(msg_type(TypeParam()), msg.type());

  EXPECT_EQ(any_msg->type(), msg.type());
  EXPECT_EQ(any_msg->length(), msg.length());
  EXPECT_EQ(any_msg->cookie(), msg.cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg.transaction_id());
}


TYPED_TEST(any_message, try_as_invalid)
{
  auto data = msg_data(TypeParam());
  data[1] = 2;

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(msg_type(TypeParam()));
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_invalid)
{
  auto data = msg_data(TypeParam());
  data[1] = 2;

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(msg_type(TypeParam())),
    std::system_error
  );
}


TYPED_TEST(any_message, try_as_success_response_valid)
{
  auto t = msg_type(TypeParam());

  auto data = msg_data(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(t.success_response().type());

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);
  EXPECT_FALSE(any_msg->is_request());
  EXPECT_TRUE(any_msg->is_success_response());
  EXPECT_FALSE(any_msg->is_error_response());
  EXPECT_FALSE(any_msg->is_indication());

  auto msg = any_msg->try_as(t.success_response());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(t.success_response(), msg->type());
}


TYPED_TEST(any_message, as_success_response_valid)
{
  auto t = msg_type(TypeParam());

  auto data = msg_data(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(t.success_response().type());

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(t.success_response());
  EXPECT_EQ(t.success_response(), msg.type());
}


TYPED_TEST(any_message, try_as_success_response_invalid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(msg_type(TypeParam()).success_response());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_success_response_invalid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(msg_type(TypeParam()).success_response()),
    std::system_error
  );
}


TYPED_TEST(any_message, try_as_error_response_valid)
{
  auto t = msg_type(TypeParam());

  auto data = msg_data(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(t.error_response().type());

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);
  EXPECT_FALSE(any_msg->is_request());
  EXPECT_FALSE(any_msg->is_success_response());
  EXPECT_TRUE(any_msg->is_error_response());
  EXPECT_FALSE(any_msg->is_indication());

  auto msg = any_msg->try_as(t.error_response());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(t.error_response(), msg->type());
}


TYPED_TEST(any_message, as_error_response_valid)
{
  auto t = msg_type(TypeParam());

  auto data = msg_data(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(t.error_response().type());

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(t.error_response());
  EXPECT_EQ(t.error_response(), msg.type());
}


TYPED_TEST(any_message, try_as_error_response_invalid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(msg_type(TypeParam()).error_response());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_error_response_invalid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(msg_type(TypeParam()).error_response()),
    std::system_error
  );
}


TYPED_TEST(any_message, try_as_indication_valid)
{
  auto t = msg_type(TypeParam());

  auto data = msg_data(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(t.indication().type());

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);
  EXPECT_FALSE(any_msg->is_request());
  EXPECT_FALSE(any_msg->is_success_response());
  EXPECT_FALSE(any_msg->is_error_response());
  EXPECT_TRUE(any_msg->is_indication());

  auto msg = any_msg->try_as(t.indication());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(t.indication(), msg->type());
}


TYPED_TEST(any_message, as_indication_valid)
{
  auto t = msg_type(TypeParam());

  auto data = msg_data(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(t.indication().type());

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(t.indication());
  EXPECT_EQ(t.indication(), msg.type());
}


TYPED_TEST(any_message, try_as_indication_invalid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(msg_type(TypeParam()).indication());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_indication_invalid)
{
  auto data = msg_data(TypeParam());
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(msg_type(TypeParam()).indication()),
    std::system_error
  );
}


}} // namespace turner_test
