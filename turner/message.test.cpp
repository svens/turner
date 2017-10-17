#include <turner/common.test.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <turner/protocol.hpp>


namespace turner_test { namespace {


using any_message = turner_test::fixture;


__turner_inline_var constexpr const auto protocol =
  turner::basic_protocol_t<turner_test::protocol_t>{};


TEST_F(any_message, insufficient_header_data)
{
  std::error_code error;
  auto msg = protocol.from_wire(raw, raw + protocol_t::header_size() / 2, error);
  EXPECT_EQ(turner::errc::insufficient_header_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    protocol.from_wire(raw, raw + protocol_t::header_size() / 2),
    std::system_error
  );
}


TEST_F(any_message, type)
{
  std::error_code error;
  auto msg = protocol.from_wire(raw, raw_end, error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(1, msg->type());
}


TEST_F(any_message, invalid_type)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  data[0] |= 0b1100'0000;

  std::error_code error;
  auto msg = protocol.from_wire(data, data_end, error);
  EXPECT_EQ(turner::errc::invalid_message_type, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    protocol.from_wire(data, data_end),
    std::system_error
  );
}


TEST_F(any_message, length)
{
  std::error_code error;
  auto msg = protocol.from_wire(raw, raw_end, error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(8, msg->length());
}


TEST_F(any_message, invalid_length)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  data[3] += 1;

  std::error_code error;
  auto msg = protocol.from_wire(data, data_end, error);
  EXPECT_EQ(turner::errc::invalid_message_length, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    protocol.from_wire(data, data_end),
    std::system_error
  );
}


TEST_F(any_message, insufficient_payload_data)
{
  std::error_code error;
  auto msg = protocol.from_wire(raw, raw_end - 1, error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    protocol.from_wire(raw, raw_end - 1),
    std::system_error
  );
}


TEST_F(any_message, invalid_cookie)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  data[protocol_t::cookie_offset()] += 1;

  std::error_code error;
  auto msg = protocol.from_wire(data, data_end, error);
  EXPECT_EQ(turner::errc::invalid_message_cookie, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    protocol.from_wire(data, data_end),
    std::system_error
  );
}


TEST_F(any_message, transaction_id)
{
  std::error_code error;
  auto msg = protocol.from_wire(raw, raw_end, error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  const std::array<uint8_t, 11> expected =
  {{
    'T', 'r', 'a', 'n', 's', 'a', 'c', 't', 'i', 'o', 'n',
  }};

  EXPECT_EQ(expected, msg->transaction_id());
}

__turner_inline_var constexpr const auto message_type =
  turner::message_type_t<turner_test::protocol_t, 1>{};


TEST_F(any_message, as_valid)
{
  auto any_msg = protocol.from_wire(raw, raw_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type);
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type, msg->message_type);

  EXPECT_EQ(any_msg, msg);
  EXPECT_EQ(any_msg->type(), msg->type());
  EXPECT_EQ(any_msg->length(), msg->length());
  EXPECT_EQ(any_msg->cookie(), msg->cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg->transaction_id());
}


TEST_F(any_message, as_invalid)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  data[1] = 2;

  auto any_msg = protocol.from_wire(data, data_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type);
  EXPECT_EQ(nullptr, msg);
}


TEST_F(any_message, as_success_response_invalid)
{
  auto any_msg = protocol.from_wire(raw, raw_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type.success_response());
  EXPECT_EQ(nullptr, msg);
}


TEST_F(any_message, as_success_response_valid)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  reinterpret_cast<uint16_t *>(data)[0] =
    sal::native_to_network_byte_order(message_type.success_response().type());

  auto any_msg = protocol.from_wire(data, data_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type.success_response());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type.success_response(), msg->message_type);
}


TEST_F(any_message, as_error_response_invalid)
{
  auto any_msg = protocol.from_wire(raw, raw_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type.error_response());
  EXPECT_EQ(nullptr, msg);
}


TEST_F(any_message, as_error_response_valid)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  reinterpret_cast<uint16_t *>(data)[0] =
    sal::native_to_network_byte_order(message_type.error_response().type());

  auto any_msg = protocol.from_wire(data, data_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type.error_response());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type.error_response(), msg->message_type);
}


TEST_F(any_message, as_indication_invalid)
{
  auto any_msg = protocol.from_wire(raw, raw_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type.indication());
  EXPECT_EQ(nullptr, msg);
}


TEST_F(any_message, as_indication_valid)
{
  char data[sizeof(raw)] = {};
  auto data_end = std::uninitialized_copy(raw, raw_end, data);
  reinterpret_cast<uint16_t *>(data)[0] =
    sal::native_to_network_byte_order(message_type.indication().type());

  auto any_msg = protocol.from_wire(data, data_end);
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->as(message_type.indication());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(message_type.indication(), msg->message_type);
}


}} // namespace turner_test
