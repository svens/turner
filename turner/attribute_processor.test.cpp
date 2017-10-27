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
using attr_t = turner::basic_attribute_type_t<
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


// error {{{1


__turner_inline_var constexpr const turner::error_t expected_error{300, "Text"};


TYPED_TEST(attribute_processor, read_error)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x07,         // Length

    0x00, 0x00, 0x03, 0x00, // Value
    'M', 's', 'g',
    0xab
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::error_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_error, value);
  EXPECT_EQ("Msg", value.message);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_error_last_attribute)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,         // Type (not expected 0x1)
    0x00, 0x00,         // Length

    0x00, 0x01,         // Type (expected 0x1)
    0x00, 0x07,         // Length

    0x00, 0x00, 0x03, 0x00, // Value
    'M', 's', 'g',
    0xab
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::error_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(expected_error, value);
  EXPECT_EQ("Msg", value.message);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_error_attribute_not_found)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,         // Type (not expected 0x1)
    0x00, 0x07,         // Length

    0x00, 0x00, 0x03, 0x00, // Value
    'M', 's', 'g',
    0xab
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::error_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_error_length_past_message_end)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x09,         // Length (past message)

    0x00, 0x00, 0x03, 0x00, // Value
    'M', 's', 'g',
    0xab
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::error_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_error_unexpected_attribute_length)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,         // Type
    0x00, 0x00,         // Length (need at least 4B)
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::error_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


// address {{{1


__turner_inline_var const auto expected_address_v4 =
  sal::net::ip::make_address("1.2.3.4");

__turner_inline_var const auto expected_address_v6 =
  sal::net::ip::make_address("102:304:506:708:90a:b0c:d0e:f10");

__turner_inline_var constexpr const uint16_t expected_port = 0x1234;


TYPED_TEST(attribute_processor, read_address_v4)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x08,                 // Length
    0x00, 0x01,                 // Value (v4 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv4
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  auto [ address, port ] = msg.read(attr, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_address_v6)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x14,                 // Length
    0x00, 0x02,                 // Value (v6 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv6
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c,
    0x0d, 0x0e, 0x0f, 0x10,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  auto [ address, port ] = msg.read(attr, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_address_v4_last_attribute)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,                 // Type (not expected 0x1)
    0x00, 0x00,                 // Length

    0x00, 0x01,                 // Type
    0x00, 0x08,                 // Length
    0x00, 0x01,                 // Value (v4 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv4
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  auto [ address, port ] = msg.read(attr, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_address_v6_last_attribute)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,                 // Type (not expected 0x1)
    0x00, 0x00,                 // Length

    0x00, 0x01,                 // Type
    0x00, 0x14,                 // Length
    0x00, 0x02,                 // Value (v6 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv6
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c,
    0x0d, 0x0e, 0x0f, 0x10,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  auto [ address, port ] = msg.read(attr, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_address_attribute_not_found)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x02,                 // Type (not expected 0x1)
    0x00, 0x08,                 // Length
    0x00, 0x01,                 // Value (v4 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv4
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_address_v4_length_past_message_end)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x09,                 // Length
    0x00, 0x01,                 // Value (v4 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv4
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_address_v6_length_past_message_end)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x15,                 // Length
    0x00, 0x02,                 // Value (v6 address)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv6
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c,
    0x0d, 0x0e, 0x0f, 0x10,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_address_v4_unexpected_attribute_length)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x14,                 // Length
    0x00, 0x01,                 // Value (claimed v4 but have v6)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv6
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c,
    0x0d, 0x0e, 0x0f, 0x10,
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_address_v6_unexpected_attribute_length)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x08,                 // Length
    0x00, 0x02,                 // Value (claimed v6 but have v4)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv4
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


TYPED_TEST(attribute_processor, read_address_unexpected_address_family)
{
  auto data = msg_with_data(TypeParam(),
  {
    0x00, 0x01,                 // Type
    0x00, 0x08,                 // Length
    0x00, 0x99,                 // Value (unexpected family)
    0x12, 0x34,                 // Port
    0x01, 0x02, 0x03, 0x04,     // IPv4
  });

  auto &msg = TypeParam::from_wire(data.begin(), data.end())
    ->as(msg_type(TypeParam()));

  attr_t<TypeParam, turner::address_attribute_processor_t> attr;
  std::error_code error;
  msg.read(attr, error);
  EXPECT_EQ(turner::errc::unexpected_address_family, error);

  EXPECT_THROW(
    msg.read(attr),
    std::system_error
  );
}


// }}}1


}} // namespace turner_test
