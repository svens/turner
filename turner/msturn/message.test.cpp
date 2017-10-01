#include <turner/common.test.hpp>
#include <turner/msturn/message.hpp>

#if __sal_os_windows
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32")
#endif


namespace turner_test { namespace {


namespace msturn = turner::msturn;
using MSTURN = msturn::protocol_t;


using msturn_message = turner_test::fixture;
using msturn_request = turner_test::with_value<turner::message_type_t>;
using msturn_indication = turner_test::with_value<turner::message_type_t>;


INSTANTIATE_TEST_CASE_P(turner, msturn_request,
  testing::Values(
    turner::msturn::allocate,
    turner::msturn::send,
    turner::msturn::set_active_destination
  )
);


INSTANTIATE_TEST_CASE_P(turner, msturn_indication,
  testing::Values(
    turner::msturn::data_indication
  )
);


TEST_P(msturn_request, request)
{
  // get request and turn it into test type
  auto buf = request(MSTURN());
  buf[1] = static_cast<uint8_t>(GetParam());

  std::error_code error;
  auto message = msturn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(GetParam(), message);
  EXPECT_EQ(message, GetParam());
  EXPECT_EQ(GetParam(), message.type());

  EXPECT_FALSE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_NE(expected_success_response(MSTURN()), message);
  EXPECT_NE(message, expected_success_response(MSTURN()));
  EXPECT_NE(expected_success_response(MSTURN()), message.type());

  EXPECT_EQ(expected_length(MSTURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(MSTURN()), message.transaction_id());

  EXPECT_NO_THROW(
    msturn::message(buf)
  );
}


TEST_P(msturn_request, success_response)
{
  if (GetParam() == turner::msturn::send)
  {
    return;
  }

  // get request and turn it into test type
  auto buf = request(MSTURN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::success_response_class);

  std::error_code error;
  auto message = msturn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_FALSE(message.is_indication());
  EXPECT_TRUE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_EQ(expected_length(MSTURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(MSTURN()), message.transaction_id());

  EXPECT_NO_THROW(
    msturn::message(buf)
  );
}


TEST_P(msturn_request, error_response)
{
  if (GetParam() == turner::msturn::send)
  {
    return;
  }

  // get request and turn it into test type
  auto buf = request(MSTURN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::error_response_class);

  std::error_code error;
  auto message = msturn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_FALSE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_TRUE(message.is_error_response());

  EXPECT_EQ(expected_length(MSTURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(MSTURN()), message.transaction_id());

  EXPECT_NO_THROW(
    msturn::message(buf)
  );
}


TEST_P(msturn_indication, indication)
{
  // get request and turn it into test type
  auto buf = request(MSTURN());
  *reinterpret_cast<uint16_t *>(buf.data()) =
    htons(GetParam() | turner::__bits::indication_class);

  std::error_code error;
  auto message = msturn::message(buf, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_TRUE(message.is_valid());

  EXPECT_EQ(GetParam(), message);
  EXPECT_EQ(message, GetParam());
  EXPECT_EQ(GetParam(), message.type());

  EXPECT_TRUE(message.is_indication());
  EXPECT_FALSE(message.is_success_response());
  EXPECT_FALSE(message.is_error_response());

  EXPECT_EQ(expected_length(MSTURN()), message.length());
  EXPECT_EQ(buf.size(), message.size());

  EXPECT_EQ(expected_transaction_id(MSTURN()), message.transaction_id());

  EXPECT_NO_THROW(
    msturn::message(buf)
  );
}


TEST_F(msturn_message, ctor)
{
  msturn::message_t message;
  EXPECT_FALSE(message.is_valid());
  EXPECT_TRUE(!message);
}


TEST_F(msturn_message, with_insufficient_data)
{
  std::array<uint8_t, 1> data = {{ 0x01 }};
  std::error_code error;
  auto message = msturn::message(data, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    msturn::message(data),
    std::system_error
  );
}


TEST_F(msturn_message, with_bad_type)
{
  std::error_code error;
  auto buf = message_with_bad_type(MSTURN());
  auto message = msturn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    msturn::message(buf),
    std::system_error
  );
}


TEST_F(msturn_message, with_bad_length)
{
  std::error_code error;
  auto buf = message_with_bad_length(MSTURN());
  auto message = msturn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    msturn::message(buf),
    std::system_error
  );
}


TEST_F(msturn_message, with_valid_length_and_insufficient_dat)
{
  std::error_code error;
  auto buf = message_with_valid_length_and_insufficient_data(MSTURN());
  auto message = msturn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    msturn::message(buf),
    std::system_error
  );
}


TEST_F(msturn_message, with_bad_cookie)
{
  std::error_code error;
  auto buf = message_with_bad_cookie(MSTURN());
  auto message = msturn::message(buf, error);
  ASSERT_FALSE(!error) << error.message();
  ASSERT_TRUE(!message);

  EXPECT_THROW(
    msturn::message(buf),
    std::system_error
  );
}


}} // namespace turner_test
