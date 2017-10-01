#include <turner/common.test.hpp>
#include <turner/stun/message.hpp>

#if __sal_os_windows
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32")
#elif __sal_os_linux
  #include <arpa/inet.h>
#endif


namespace turner_test { namespace {


namespace stun = turner::stun;
using STUN = stun::protocol_t;


using stun_message = turner_test::fixture;
using stun_request = turner_test::with_value<turner::message_type_t>;


INSTANTIATE_TEST_CASE_P(turner, stun_request,
  testing::Values(
    turner::stun::binding
  )
);


TEST_P(stun_request, request)
{
  // get request and turn it into test type
  auto buf = request(STUN());
  buf[1] = static_cast<uint8_t>(GetParam());

  std::error_code error;
  auto message = stun::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(GetParam(), message);
  EXPECT_EQ(message, GetParam());
  EXPECT_EQ(GetParam(), message.type());

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


TEST_P(stun_request, success_response)
{
  // get request and turn it into test type
  auto buf = request(STUN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::success_response_class);

  std::error_code error;
  auto message = stun::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_FALSE(message.is_indication());
  EXPECT_TRUE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_EQ(expected_length(STUN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(STUN()), message.transaction_id());

  EXPECT_NO_THROW(
    stun::message(buf)
  );
}


TEST_F(stun_message, ctor)
{
  stun::message_t message;
  EXPECT_FALSE(message.is_valid());
  EXPECT_TRUE(!message);
}


TEST_F(stun_message, with_insufficient_data)
{
  std::array<uint8_t, 1> data = {{ 0x01 }};
  std::error_code error;
  auto message = stun::message(data, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(data),
    std::system_error
  );
}


TEST_F(stun_message, with_bad_type)
{
  std::error_code error;
  auto buf = message_with_bad_type(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


TEST_F(stun_message, with_bad_length)
{
  std::error_code error;
  auto buf = message_with_bad_length(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


TEST_F(stun_message, with_valid_length_and_insufficient_dat)
{
  std::error_code error;
  auto buf = message_with_valid_length_and_insufficient_data(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


TEST_F(stun_message, with_bad_cookie)
{
  std::error_code error;
  auto buf = message_with_bad_cookie(STUN());
  auto message = stun::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    stun::message(buf),
    std::system_error
  );
}


}} // namespace turner_test
