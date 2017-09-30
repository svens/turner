#include <turner/common.test.hpp>
#include <turner/stun/message.hpp>
#include <turner/error.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using turner_message = turner_test::with_type<Protocol>;

TYPED_TEST_CASE(turner_message, protocol_types);


TYPED_TEST(turner_message, ctor)
{
  turner::basic_message_t<TypeParam> message;
  EXPECT_FALSE(message.is_valid());
  EXPECT_TRUE(!message);
}


TYPED_TEST(turner_message, make)
{
  std::error_code error;

  auto buf = request(TypeParam());
  auto message = turner::basic_message_t<TypeParam>::make(
    buf.data(), buf.data() + buf.size(), error
  );

  ASSERT_TRUE(!error);
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(expected_request(TypeParam()), message);
  EXPECT_EQ(message, expected_request(TypeParam()));
  EXPECT_EQ(expected_request(TypeParam()), message.type());

  EXPECT_FALSE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_NE(expected_success_response(TypeParam()), message);
  EXPECT_NE(message, expected_success_response(TypeParam()));
  EXPECT_NE(expected_success_response(TypeParam()), message.type());

  EXPECT_EQ(expected_length(TypeParam()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(TypeParam()), message.transaction_id());
}


TYPED_TEST(turner_message, make_success_response)
{
  std::error_code error;

  auto buf = success_response(TypeParam());
  auto message = turner::basic_message_t<TypeParam>::make(
    buf.data(), buf.data() + buf.size(), error
  );

  ASSERT_TRUE(!error);
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(expected_success_response(TypeParam()), message);
  EXPECT_EQ(message, expected_success_response(TypeParam()));
  EXPECT_EQ(expected_success_response(TypeParam()), message.type());
  EXPECT_FALSE(message.is_indication());
  EXPECT_TRUE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_EQ(expected_length(TypeParam()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(TypeParam()), message.transaction_id());
}


TYPED_TEST(turner_message, make_with_invalid_begin)
{
  std::error_code error;

  uint8_t data{};
  auto message = turner::basic_message_t<TypeParam>::make(
    {}, &data, error
  );

  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_invalid_end)
{
  std::error_code error;

  uint8_t data{};
  auto message = turner::basic_message_t<TypeParam>::make(
    &data, {}, error
  );

  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_begin_gt_end)
{
  std::error_code error;

  uint8_t data{};
  auto message = turner::basic_message_t<TypeParam>::make(
    &data + sizeof(data), &data, error
  );

  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_insufficient_data)
{
  std::error_code error;

  uint8_t data = 0x01;
  auto message = turner::basic_message_t<TypeParam>::make(
    &data, &data + sizeof(data), error
  );

  EXPECT_EQ(turner::errc::insufficient_data, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_bad_type)
{
  std::error_code error;

  auto buf = message_with_bad_type(TypeParam());
  auto message = turner::basic_message_t<TypeParam>::make(
    buf.data(), buf.data() + buf.size(), error
  );

  EXPECT_EQ(turner::errc::invalid_message_header, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_bad_length)
{
  std::error_code error;

  auto buf = message_with_bad_length(TypeParam());
  auto message = turner::basic_message_t<TypeParam>::make(
    buf.data(), buf.data() + buf.size(), error
  );

  EXPECT_EQ(turner::errc::invalid_message_length, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_valid_length_and_insufficient_data)
{
  std::error_code error;

  auto buf = message_with_valid_length_and_insufficient_data(TypeParam());
  auto message = turner::basic_message_t<TypeParam>::make(
    buf.data(), buf.data() + buf.size(), error
  );

  EXPECT_EQ(turner::errc::insufficient_data, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, make_with_bad_cookie)
{
  std::error_code error;

  auto buf = message_with_bad_cookie(TypeParam());
  auto message = turner::basic_message_t<TypeParam>::make(
    buf.data(), buf.data() + buf.size(), error
  );

  EXPECT_EQ(turner::errc::invalid_message_header, error);
  EXPECT_FALSE(message.is_valid());
}


}} // namespace turner_test
