#include <turner/common.test.hpp>
#include <turner/turn/message.hpp>

#if __sal_os_windows
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32")
#elif __sal_os_linux
  #include <arpa/inet.h>
#endif


namespace turner_test { namespace {


namespace turn = turner::turn;
using TURN = turn::protocol_t;


using turn_message = turner_test::fixture;
using turn_request = turner_test::with_value<turner::message_type_t>;
using turn_indication = turner_test::with_value<turner::message_type_t>;


INSTANTIATE_TEST_CASE_P(turner, turn_request,
  testing::Values(
    turner::stun::binding,
    turner::turn::allocation,
    turner::turn::refresh,
    turner::turn::create_permission,
    turner::turn::channel_bind
  )
);


INSTANTIATE_TEST_CASE_P(turner, turn_indication,
  testing::Values(
    turner::turn::send_indication,
    turner::turn::data_indication
  )
);


TEST_P(turn_request, request)
{
  // get request and turn it into test type
  auto buf = request(TURN());
  buf[1] = static_cast<uint8_t>(GetParam());

  std::error_code error;
  auto message = turn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(GetParam(), message);
  EXPECT_EQ(message, GetParam());
  EXPECT_EQ(GetParam(), message.type());

  EXPECT_FALSE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_NE(expected_success_response(TURN()), message);
  EXPECT_NE(message, expected_success_response(TURN()));
  EXPECT_NE(expected_success_response(TURN()), message.type());

  EXPECT_EQ(expected_length(TURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(TURN()), message.transaction_id());

  EXPECT_NO_THROW(
    turn::message(buf)
  );
}


TEST_P(turn_request, success_response)
{
  // get request and turn it into test type
  auto buf = request(TURN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::success_response_class);

  std::error_code error;
  auto message = turn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_FALSE(message.is_indication());
  EXPECT_TRUE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_EQ(expected_length(TURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(TURN()), message.transaction_id());

  EXPECT_NO_THROW(
    turn::message(buf)
  );
}


TEST_P(turn_request, error_response)
{
  if (GetParam() == turner::stun::binding)
  {
    // no STUN Binding error response
    return;
  }

  // get request and turn it into test type
  auto buf = request(TURN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::error_response_class);

  std::error_code error;
  auto message = turn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_FALSE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_TRUE(message.is_error_response());

  EXPECT_EQ(expected_length(TURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(TURN()), message.transaction_id());

  EXPECT_NO_THROW(
    turn::message(buf)
  );
}


TEST_P(turn_indication, indication)
{
  // get request and turn it into test type
  auto buf = request(TURN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::indication_class);

  std::error_code error;
  auto message = turn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(GetParam(), message);
  EXPECT_EQ(message, GetParam());
  EXPECT_EQ(GetParam(), message.type());

  EXPECT_TRUE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_EQ(expected_length(TURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(TURN()), message.transaction_id());

  EXPECT_NO_THROW(
    turn::message(buf)
  );
}


TEST_F(turn_message, ctor)
{
  turn::message_t message;
  EXPECT_FALSE(message.is_valid());
  EXPECT_TRUE(!message);
}


TEST_F(turn_message, with_insufficient_data)
{
  std::array<uint8_t, 1> data = {{ 0x01 }};
  std::error_code error;
  auto message = turn::message(data, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    turn::message(data),
    std::system_error
  );
}


TEST_F(turn_message, with_bad_type)
{
  std::error_code error;
  auto buf = message_with_bad_type(TURN());
  auto message = turn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    turn::message(buf),
    std::system_error
  );
}


TEST_F(turn_message, with_bad_length)
{
  std::error_code error;
  auto buf = message_with_bad_length(TURN());
  auto message = turn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    turn::message(buf),
    std::system_error
  );
}


TEST_F(turn_message, with_valid_length_and_insufficient_dat)
{
  std::error_code error;
  auto buf = message_with_valid_length_and_insufficient_data(TURN());
  auto message = turn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    turn::message(buf),
    std::system_error
  );
}


TEST_F(turn_message, with_bad_cookie)
{
  std::error_code error;
  auto buf = message_with_bad_cookie(TURN());
  auto message = turn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    turn::message(buf),
    std::system_error
  );
}


}} // namespace turner_test
