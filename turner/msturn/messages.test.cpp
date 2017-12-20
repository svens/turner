#include <turner/common.test.hpp>


namespace turner_test { namespace {


using msturn = turner_test::fixture;


TEST_F(msturn, allocate)
{
  EXPECT_EQ(0x0003, turner::msturn::allocate.type());
  EXPECT_TRUE(turner::msturn::allocate.is_request());

  EXPECT_EQ(0x0103, turner::msturn::allocate_success.type());
  EXPECT_TRUE(turner::msturn::allocate_success.is_success_response());

  EXPECT_EQ(0x0113, turner::msturn::allocate_error.type());
  EXPECT_TRUE(turner::msturn::allocate_error.is_error_response());
}


TEST_F(msturn, send_name)
{
  EXPECT_EQ(0x0004, turner::msturn::send.type());
  EXPECT_TRUE(turner::msturn::send.is_request());
}


TEST_F(msturn, data_indication)
{
  EXPECT_EQ(0x0115, turner::msturn::data_indication.type());
  EXPECT_TRUE(turner::msturn::data_indication.is_indication());
  EXPECT_FALSE(turner::msturn::data_indication.is_error_response());
}


TEST_F(msturn, set_active_destination)
{
  EXPECT_EQ(0x0006, turner::msturn::set_active_destination.type());
  EXPECT_TRUE(turner::msturn::set_active_destination.is_request());

  EXPECT_EQ(0x0106, turner::msturn::set_active_destination_success.type());
  EXPECT_TRUE(turner::msturn::set_active_destination_success.is_success_response());

  EXPECT_EQ(0x0116, turner::msturn::set_active_destination_error.type());
  EXPECT_TRUE(turner::msturn::set_active_destination_error.is_error_response());
}


TEST_F(msturn, finish_with_integrity)
{
  auto expected = MSTURN::msg_data();

  // instantiate writer with data area
  std::vector<uint8_t> data(expected.size(), 0);
  std::error_code error;
  auto writer = turner::msturn::protocol.build(turner::msturn::allocate,
    data.begin(), data.end()
  );

  // instead of actually building message, copy expeted message without
  // Message Integrity
  std::uninitialized_copy_n(expected.cbegin(),
    expected.size() - 24,
    data.begin()
  );
  data[3] -= 24;
  EXPECT_EQ(MSTURN::min_payload_length(), writer.length());

  // actual testing part: finish with integrity
  auto integrity_calculator = MSTURN::msg_hmac();
  auto [ begin, end ] = writer.finish(integrity_calculator);
  EXPECT_EQ(data.data(), begin);
  EXPECT_EQ(data.data() + data.size(), end);
  EXPECT_EQ(expected, data);
}


TEST_F(msturn, finish_with_integrity_v3)
{
  std::vector<uint8_t> expected =
  {{
    0x00, 0x03, 0x00, 0x2c,     // Type & Length
    0x00, 0x01, 0x02, 0x03,     // Transaction ID
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x0f, 0x00, 0x04,     // Magic Cookie
    0x72, 0xc6, 0x4b, 0xc6,
    0x00, 0x08, 0x00, 0x20,     // Message Integrity (32B HMAC-SHA256)
    0x89, 0x0e, 0x90, 0xe0,
    0xfe, 0xe6, 0xed, 0x6f,
    0xa3, 0x3a, 0xc9, 0x7e,
    0xa0, 0x04, 0x76, 0x16,
    0x27, 0x95, 0xe2, 0x5a,
    0x60, 0x73, 0x86, 0xcc,
    0x98, 0x17, 0xfa, 0x6e,
    0xd8, 0xd2, 0xcd, 0x14,
  }};

  // instantiate writer with data area
  std::vector<uint8_t> data(expected.size(), 0);
  std::error_code error;
  auto writer = turner::msturn::protocol.build(turner::msturn::allocate,
    data.begin(), data.end()
  );

  // instead of actually building message, copy expeted message without
  // Message Integrity
  std::uninitialized_copy_n(expected.cbegin(),
    expected.size() - 36,
    data.begin()
  );
  data[3] -= 36;
  EXPECT_EQ(MSTURN::min_payload_length(), writer.length());

  auto integrity_calculator = turner::msturn::make_integrity_calculator_v3(
    "realm",
    "nonce",
    "user",
    "pass"
  );
  auto [ begin, end ] = writer.finish(integrity_calculator);
  EXPECT_EQ(data.data(), begin);
  EXPECT_EQ(data.data() + data.size(), end);
  EXPECT_EQ(expected, data);
}


}} // namespace turner_test
