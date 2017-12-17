#include <turner/common.test.hpp>


namespace turner_test { namespace {


using msturn = turner_test::fixture;


TEST_F(msturn, allocate)
{
  EXPECT_STREQ("allocate", turner::msturn::allocate.name());
  EXPECT_EQ(0x0003, turner::msturn::allocate.type());
  EXPECT_TRUE(turner::msturn::allocate.is_request());

  EXPECT_STREQ("allocate_success", turner::msturn::allocate_success.name());
  EXPECT_EQ(0x0103, turner::msturn::allocate_success.type());
  EXPECT_TRUE(turner::msturn::allocate_success.is_success_response());

  EXPECT_STREQ("allocate_error", turner::msturn::allocate_error.name());
  EXPECT_EQ(0x0113, turner::msturn::allocate_error.type());
  EXPECT_TRUE(turner::msturn::allocate_error.is_error_response());
}


TEST_F(msturn, send_name)
{
  EXPECT_STREQ("send", turner::msturn::send.name());
  EXPECT_EQ(0x0004, turner::msturn::send.type());
  EXPECT_TRUE(turner::msturn::send.is_request());
}


TEST_F(msturn, data_indication)
{
  EXPECT_STREQ("data_indication", turner::msturn::data_indication.name());
  EXPECT_EQ(0x0115, turner::msturn::data_indication.type());
  EXPECT_TRUE(turner::msturn::data_indication.is_indication());
  EXPECT_FALSE(turner::msturn::data_indication.is_error_response());
}


TEST_F(msturn, set_active_destination)
{
  EXPECT_STREQ("set_active_destination",
    turner::msturn::set_active_destination.name()
  );
  EXPECT_EQ(0x0006, turner::msturn::set_active_destination.type());
  EXPECT_TRUE(turner::msturn::set_active_destination.is_request());

  EXPECT_STREQ("set_active_destination_success",
    turner::msturn::set_active_destination_success.name()
  );
  EXPECT_EQ(0x0106, turner::msturn::set_active_destination_success.type());
  EXPECT_TRUE(turner::msturn::set_active_destination_success.is_success_response());

  EXPECT_STREQ("set_active_destination_error",
    turner::msturn::set_active_destination_error.name()
  );
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


}} // namespace turner_test
