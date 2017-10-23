#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using any_message = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(any_message, protocol_types);


TYPED_TEST(any_message, insufficient_header_data)
{
  auto data = x_request(TypeParam());
  data.resize(data.size() / 2);

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_header_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::from_wire(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, type)
{
  auto data = x_request(TypeParam());

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(x_message_type_value(TypeParam()), msg->type());
}


TYPED_TEST(any_message, invalid_type)
{
  auto data = x_request(TypeParam());
  data[0] |= 0b1100'0000;

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_type, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::from_wire(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, length)
{
  auto data = x_request(TypeParam());

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(x_message_length(TypeParam()), msg->length());
}


TYPED_TEST(any_message, invalid_length)
{
  auto data = x_request(TypeParam());
  data[3] += 1;

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_length, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::from_wire(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, insufficient_payload_data)
{
  auto data = x_request(TypeParam());
  data.pop_back();

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::from_wire(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, invalid_cookie)
{
  auto data = x_request(TypeParam());
  data[TypeParam::traits_t::cookie_offset] += 1;

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_cookie, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::from_wire(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, transaction_id)
{
  auto data = x_request(TypeParam());

  std::error_code error;
  auto msg = TypeParam::from_wire(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(x_transaction_id(TypeParam()), msg->transaction_id());
}


TYPED_TEST(any_message, try_as_valid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(x_message_type(TypeParam()));
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(x_message_type(TypeParam()), msg->message_type);

  EXPECT_EQ(any_msg, msg);
  EXPECT_EQ(any_msg->type(), msg->type());
  EXPECT_EQ(any_msg->length(), msg->length());
  EXPECT_EQ(any_msg->cookie(), msg->cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg->transaction_id());
}


TYPED_TEST(any_message, as_valid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(x_message_type(TypeParam()));
  EXPECT_EQ(x_message_type(TypeParam()), msg.message_type);

  EXPECT_EQ(any_msg->type(), msg.type());
  EXPECT_EQ(any_msg->length(), msg.length());
  EXPECT_EQ(any_msg->cookie(), msg.cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg.transaction_id());
}


TYPED_TEST(any_message, try_as_invalid)
{
  auto data = x_request(TypeParam());
  data[1] = 2;

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(x_message_type(TypeParam()));
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_invalid)
{
  auto data = x_request(TypeParam());
  data[1] = 2;

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(x_message_type(TypeParam())),
    std::system_error
  );
}


TYPED_TEST(any_message, try_as_success_response_valid)
{
  auto message_type = x_message_type(TypeParam());

  auto data = x_request(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(message_type.success_response().type());

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(message_type.success_response());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type.success_response(), msg->message_type);
}


TYPED_TEST(any_message, as_success_response_valid)
{
  auto message_type = x_message_type(TypeParam());

  auto data = x_request(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(message_type.success_response().type());

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(message_type.success_response());
  EXPECT_EQ(message_type.success_response(), msg.message_type);
}


TYPED_TEST(any_message, try_as_success_response_invalid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(x_message_type(TypeParam()).success_response());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_success_response_invalid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(x_message_type(TypeParam()).success_response()),
    std::system_error
  );
}


TYPED_TEST(any_message, try_as_error_response_valid)
{
  auto message_type = x_message_type(TypeParam());

  auto data = x_request(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(message_type.error_response().type());

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(message_type.error_response());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type.error_response(), msg->message_type);
}


TYPED_TEST(any_message, as_error_response_valid)
{
  auto message_type = x_message_type(TypeParam());

  auto data = x_request(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(message_type.error_response().type());

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(message_type.error_response());
  EXPECT_EQ(message_type.error_response(), msg.message_type);
}


TYPED_TEST(any_message, try_as_error_response_invalid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(x_message_type(TypeParam()).error_response());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_error_response_invalid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(x_message_type(TypeParam()).error_response()),
    std::system_error
  );
}


TYPED_TEST(any_message, try_as_indication_valid)
{
  auto message_type = x_message_type(TypeParam());

  auto data = x_request(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(message_type.indication().type());

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(message_type.indication());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type.indication(), msg->message_type);
}


TYPED_TEST(any_message, as_indication_valid)
{
  auto message_type = x_message_type(TypeParam());

  auto data = x_request(TypeParam());
  reinterpret_cast<uint16_t *>(data.data())[0] =
    sal::native_to_network_byte_order(message_type.indication().type());

  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(message_type.indication());
  EXPECT_EQ(message_type.indication(), msg.message_type);
}


TYPED_TEST(any_message, try_as_indication_invalid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(x_message_type(TypeParam()).indication());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_indication_invalid)
{
  auto data = x_request(TypeParam());
  auto any_msg = TypeParam::from_wire(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(x_message_type(TypeParam()).indication()),
    std::system_error
  );
}


}} // namespace turner_test
