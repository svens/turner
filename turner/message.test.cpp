#include <turner/common.test.hpp>


namespace turner_test { namespace {


// any_message {{{1


template <typename Protocol>
using any_message = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(any_message, protocol_types);


TYPED_TEST(any_message, no_data)
{
  std::vector<uint8_t> data;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_header_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, insufficient_header_data)
{
  auto data = TypeParam::msg_data();
  data.resize(TypeParam::traits_t::header_size - 2);

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_header_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, type)
{
  auto data = TypeParam::msg_data();

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(TypeParam::msg_type_v(), msg->type());
  EXPECT_TRUE(msg->is_request());
  EXPECT_FALSE(msg->is_success_response());
  EXPECT_FALSE(msg->is_error_response());
  EXPECT_FALSE(msg->is_indication());
}


TYPED_TEST(any_message, invalid_type)
{
  auto data = TypeParam::msg_data();
  data[0] |= 0b1100'0000;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_type, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, length)
{
  auto data = TypeParam::msg_data();

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);
  EXPECT_EQ(TypeParam::msg_len(), msg->length());
}


TYPED_TEST(any_message, invalid_length)
{
  if constexpr (TypeParam::traits_t::padding_size < 2)
  {
    // no padding: no invalid length
    return;
  }

  auto data = TypeParam::msg_data();
  data[3] += 1;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_length, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, insufficient_payload_data)
{
  auto data = TypeParam::msg_data();
  data.pop_back();

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, invalid_cookie)
{
  auto data = TypeParam::msg_data();
  data[TypeParam::traits_t::cookie_offset] += 1;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_EQ(turner::errc::invalid_message_cookie, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    TypeParam::parse(data.begin(), data.end()),
    std::system_error
  );
}


TYPED_TEST(any_message, transaction_id)
{
  auto data = TypeParam::msg_data();

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(TypeParam::msg_txn_id(), msg->transaction_id());
}


TYPED_TEST(any_message, try_as_valid)
{
  auto data = TypeParam::msg_data();
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(TypeParam::msg_type());
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(TypeParam::msg_type(), msg->type());

  EXPECT_EQ(any_msg, msg);
  EXPECT_EQ(any_msg->type(), msg->type());
  EXPECT_EQ(any_msg->length(), msg->length());
  EXPECT_EQ(any_msg->cookie(), msg->cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg->transaction_id());
}


TYPED_TEST(any_message, as_valid)
{
  auto data = TypeParam::msg_data();
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto &msg = any_msg->as(TypeParam::msg_type());
  EXPECT_EQ(TypeParam::msg_type(), msg.type());

  EXPECT_EQ(any_msg->type(), msg.type());
  EXPECT_EQ(any_msg->length(), msg.length());
  EXPECT_EQ(any_msg->cookie(), msg.cookie());
  EXPECT_EQ(any_msg->transaction_id(), msg.transaction_id());
}


TYPED_TEST(any_message, try_as_invalid)
{
  auto data = TypeParam::msg_data();
  data[1] = 2;

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  auto msg = any_msg->try_as(TypeParam::msg_type());
  EXPECT_EQ(nullptr, msg);
}


TYPED_TEST(any_message, as_invalid)
{
  auto data = TypeParam::msg_data();
  data[1] = 2;

  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  EXPECT_THROW(
    any_msg->as(TypeParam::msg_type()),
    std::system_error
  );
}


TYPED_TEST(any_message, has_valid_integrity)
{
  auto data = TypeParam::msg_data();
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  std::error_code error;
  auto integrity_calculator = TypeParam::msg_hmac();
  EXPECT_TRUE(any_msg->has_valid_integrity(integrity_calculator, error));
  EXPECT_TRUE(!error) << error.message();
}


TYPED_TEST(any_message, has_valid_integrity_attribute_not_found)
{
  auto data = wire_data(TypeParam(), "");
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  std::error_code error;
  auto integrity_calculator = TypeParam::msg_hmac();
  EXPECT_FALSE(any_msg->has_valid_integrity(integrity_calculator, error));
  EXPECT_EQ(turner::errc::attribute_not_found, error);
}


TYPED_TEST(any_message, has_valid_integrity_unexpected_attribute_length)
{
  auto data = TypeParam::msg_data();
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  std::error_code error;
  sal::crypto::hmac_t<sal::crypto::md5> integrity_calculator;
  EXPECT_FALSE(any_msg->has_valid_integrity(integrity_calculator, error));
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);
}


TYPED_TEST(any_message, has_valid_integrity_unexpected_value)
{
  auto data = TypeParam::msg_data();
  data.back() ^= 1;
  auto any_msg = TypeParam::parse(data.begin(), data.end());
  ASSERT_TRUE(any_msg);

  std::error_code error;
  auto integrity_calculator = TypeParam::msg_hmac();
  EXPECT_FALSE(any_msg->has_valid_integrity(integrity_calculator, error));
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
}


// message_reader {{{1


template <typename Protocol>
using message_reader = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(message_reader, protocol_types);


TYPED_TEST(message_reader, to_success_response_new_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() + 1);

  std::error_code error;
  auto writer = msg.to_success_response(new_data.begin(), new_data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_success_type(), writer.type());
  EXPECT_EQ(1, writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(
    msg.to_success_response(new_data.begin(), new_data.end())
  );
}


TYPED_TEST(message_reader, to_success_response_new_region_not_enough_room)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() - 1);

  std::error_code error;
  auto writer = msg.to_success_response(new_data.begin(), new_data.end(), error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);

  EXPECT_THROW(
    msg.to_success_response(new_data.begin(), new_data.end()),
    std::system_error
  );
}


TYPED_TEST(message_reader, to_success_response_same_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_success_response(data.begin(), data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_success_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size(), writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(
    msg.to_success_response(data.begin(), data.end())
  );
}


TYPED_TEST(message_reader, to_success_response_same_region_not_enough_room)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_success_response(
    data.begin(), data.begin() + TypeParam::header_and_cookie_size() - 2,
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);

  EXPECT_THROW(
    msg.to_success_response(
      data.begin(),
      data.begin() + TypeParam::header_and_cookie_size() - 2
    ),
    std::system_error
  );
}


TYPED_TEST(message_reader, to_success_response_overlapped_region_left)
{
  auto data = TypeParam::msg_data();
  data.insert(data.begin(), 0);
  auto &msg = TypeParam::parse(data.begin() + 1, data.end())
    ->as(TypeParam::msg_type());

  std::error_code error;
  auto writer = msg.to_success_response(data.begin(), data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_success_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size(), writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());
}


TYPED_TEST(message_reader, to_success_response_overlapped_region_right)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_success_response(data.begin() + 1, data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_success_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size() - 1, writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());
}


TYPED_TEST(message_reader, to_success_response_data_new_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() + 1);

  std::error_code error;
  auto writer = msg.to_success_response(new_data, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_success_type(), writer.type());
  EXPECT_EQ(1, writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(msg.to_success_response(new_data));
}


TYPED_TEST(message_reader, to_success_response_data_new_region_not_enough_room)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() - 1);

  std::error_code error;
  auto writer = msg.to_success_response(new_data, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);

  EXPECT_THROW(
    msg.to_success_response(new_data),
    std::system_error
  );
}


TYPED_TEST(message_reader, to_success_response_data_same_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_success_response(data, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_success_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size(), writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(msg.to_success_response(data));
}


TYPED_TEST(message_reader, to_error_response_new_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() + 1);

  std::error_code error;
  auto writer = msg.to_error_response(new_data.begin(), new_data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_error_type(), writer.type());
  EXPECT_EQ(1, writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(
    msg.to_error_response(new_data.begin(), new_data.end())
  );
}


TYPED_TEST(message_reader, to_error_response_new_region_not_enough_room)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() - 1);

  std::error_code error;
  auto writer = msg.to_error_response(new_data.begin(), new_data.end(), error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);

  EXPECT_THROW(
    msg.to_error_response(new_data.begin(), new_data.end()),
    std::system_error
  );
}


TYPED_TEST(message_reader, to_error_response_same_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_error_response(data.begin(), data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_error_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size(), writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(
    msg.to_error_response(data.begin(), data.end())
  );
}


TYPED_TEST(message_reader, to_error_response_same_region_not_enough_room)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_error_response(
    data.begin(), data.begin() + TypeParam::header_and_cookie_size() - 2,
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);

  EXPECT_THROW(
    msg.to_error_response(
      data.begin(),
      data.begin() + TypeParam::header_and_cookie_size() - 2
    ),
    std::system_error
  );
}


TYPED_TEST(message_reader, to_error_response_overlapped_region_left)
{
  auto data = TypeParam::msg_data();
  data.insert(data.begin(), 0);
  auto &msg = TypeParam::parse(data.begin() + 1, data.end())
    ->as(TypeParam::msg_type());

  std::error_code error;
  auto writer = msg.to_error_response(data.begin(), data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_error_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size(), writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());
}


TYPED_TEST(message_reader, to_error_response_overlapped_region_right)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_error_response(data.begin() + 1, data.end(), error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_error_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size() - 1, writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());
}


TYPED_TEST(message_reader, to_error_response_data_new_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() + 1);

  std::error_code error;
  auto writer = msg.to_error_response(new_data, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_error_type(), writer.type());
  EXPECT_EQ(1, writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(msg.to_error_response(new_data));
}


TYPED_TEST(message_reader, to_error_response_data_new_region_not_enough_room)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::vector<uint8_t> new_data(TypeParam::header_and_cookie_size() - 1);

  std::error_code error;
  auto writer = msg.to_error_response(new_data, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(!writer);

  EXPECT_THROW(
    msg.to_error_response(new_data),
    std::system_error
  );
}


TYPED_TEST(message_reader, to_error_response_data_same_region)
{
  auto data = TypeParam::msg_data();
  auto &msg = parse(TypeParam(), data);

  std::error_code error;
  auto writer = msg.to_error_response(data, error);
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(!writer);
  EXPECT_EQ(TypeParam::msg_error_type(), writer.type());
  EXPECT_EQ(data.size() - TypeParam::header_and_cookie_size(), writer.available());
  EXPECT_EQ(TypeParam::traits_t::cookie, writer.cookie());
  EXPECT_EQ(TypeParam::msg_txn_id(), writer.transaction_id());

  EXPECT_NO_THROW(
    msg.to_error_response(data)
  );
}


// }}}1


}} // namespace turner_test
