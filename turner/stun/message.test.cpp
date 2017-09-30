#include <turner/common.test.hpp>
#include <turner/stun/message.hpp>


namespace turner_test { namespace {


namespace stun = turner::stun;
using STUN = stun::protocol_t;


using turner_stun_message = turner_test::fixture;


TEST_F(turner_stun_message, ctor)
{
  stun::message_t message;
  EXPECT_FALSE(message.is_valid());
  EXPECT_TRUE(!message);
}


TEST_F(turner_stun_message, basic)
{
  std::error_code error;
  auto buf = request(STUN());
  auto message = stun::message(buf, error);
  ASSERT_TRUE(!error);
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(expected_request(STUN()), message);
  EXPECT_EQ(message, expected_request(STUN()));
  EXPECT_EQ(expected_request(STUN()), message.type());

  EXPECT_FALSE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_NE(expected_success_response(STUN()), message);
  EXPECT_NE(message, expected_success_response(STUN()));
  EXPECT_NE(expected_success_response(STUN()), message.type());

  EXPECT_EQ(expected_length(STUN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(STUN()), message.transaction_id());

  EXPECT_NO_THROW(
    stun::message(buf)
  );
}


TEST_F(turner_stun_message, with_insufficient_data)
{
  std::array<uint8_t, 1> data = {{ 0x01 }};
  std::error_code error;
  auto message = stun::message(data, error);
  ASSERT_FALSE(!error);
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(data),
    std::system_error
  );
}


TEST_F(turner_stun_message, with_bad_type)
{
  std::error_code error;
  auto buf = message_with_bad_type(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error);
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


TEST_F(turner_stun_message, with_bad_length)
{
  std::error_code error;
  auto buf = message_with_bad_length(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error);
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


TEST_F(turner_stun_message, with_valid_length_and_insufficient_dat)
{
  std::error_code error;
  auto buf = message_with_valid_length_and_insufficient_data(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error);
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


TEST_F(turner_stun_message, with_bad_cookie)
{
  std::error_code error;
  auto buf = message_with_bad_cookie(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error);
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


}} // namespace turner_test
