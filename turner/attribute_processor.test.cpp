#include <turner/common.test.hpp>
#include <turner/attribute.hpp>
#include <turner/attribute_processor.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using attribute_processor = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(attribute_processor, protocol_types);


template <typename Protocol>
auto msg_with_data (Protocol, std::initializer_list<uint8_t> data)
{
  // get message and remove existing body
  auto raw = msg_data(Protocol());
  raw.resize(Protocol::traits_t::header_size);

  // append new body and update message length
  raw.insert(raw.end(), data.begin(), data.end());
  reinterpret_cast<uint16_t *>(&raw[0])[1] =
    sal::native_to_network_byte_order((uint16_t)data.size());

  return raw;
}


template <
  typename Protocol,
  template <typename> typename AttributeProcessor
>
using attr_t = turner::basic_attribute_t<
  typename Protocol::traits_t,
  0x01,
  AttributeProcessor<typename Protocol::traits_t>
>;


// uint32 {{{1


TYPED_TEST(attribute_processor, read_uint32)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01, // Type
    0x00, 0x04, // Length
    0x12, 0x34, // Value
    0x56, 0x78,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0x12345678U, value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_uint32_last_attribute)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02, // Type (not expected 0x1)
    0x00, 0x00, // Length

    0x00, 0x01, // Type (expected 0x1)
    0x00, 0x04, // Length
    0x12, 0x34, // Value
    0x56, 0x78,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0x12345678U, value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_uint32_attribute_not_found)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02, // Type (not expected 0x1)
    0x00, 0x04, // Length
    0x12, 0x34, // Value
    0x56, 0x78,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_uint32_length_past_message_end)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01, // Type
    0x00, 0x08, // Length (past message)
    0x12, 0x34, // Value
    0x56, 0x78,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_uint32_unexpected_attribute_length)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01, // Type
    0x00, 0x03, // Length (!= sizeof(uint32_t))
    0x12, 0x34, // Value
    0x56, 0x78,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


// string {{{1


TYPED_TEST(attribute_processor, read_string)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x03,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_string_last_attribute)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,         // Type (not expected 0x1)
    0x00, 0x00,         // Length

    0x00, 0x01,         // Type (expected 0x1)
    0x00, 0x03,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_string_attribute_not_found)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,         // Type (not expected 0x1)
    0x00, 0x03,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_string_length_past_message_end)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x05,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_string_empty)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01, // Type
    0x00, 0x00, // Length
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("", value);

  EXPECT_NO_THROW(msg.read(attr));
}


// array {{{1


TYPED_TEST(attribute_processor, read_array)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x03,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  auto [ begin, end ] = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", std::string(begin, end));

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_array_last_attribute)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,         // Type (not expected 0x1)
    0x00, 0x00,         // Length

    0x00, 0x01,         // Type (expected 0x1)
    0x00, 0x03,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  auto [ begin, end ] = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", std::string(begin, end));

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_array_attribute_not_found)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,         // Type (not expected 0x1)
    0x00, 0x03,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_array_length_past_message_end)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x05,         // Length
    's', 't', 'r',      // Value + padding
    0x00,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_array_empty)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01, // Type
    0x00, 0x00, // Length
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  auto [ begin, end ] = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(begin, end);

  EXPECT_NO_THROW(msg.read(attr));
}


// }}}1


}} // namespace turner_test
