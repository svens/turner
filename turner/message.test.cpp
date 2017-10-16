#include <turner/common.test.hpp>
#include <turner/message.hpp>
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


}} // namespace turner_test
