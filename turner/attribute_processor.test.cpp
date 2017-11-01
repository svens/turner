#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using attribute_processor = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(attribute_processor, protocol_types);


template <typename Protocol, size_t N>
auto wire_data (Protocol, const char (&data)[N])
{
  // get message and remove existing body
  auto raw = msg_data(Protocol());
  raw.resize(Protocol::traits_t::header_size);

  // append new body and update message length
  raw.insert(raw.end(), data, data + N - 1);
  reinterpret_cast<uint16_t *>(&raw[0])[1] =
    sal::native_to_network_byte_order((uint16_t)(N - 1));

  return raw;
}


template <typename Protocol, typename Data>
inline auto &from_wire (Protocol p, const Data &d)
{
  return Protocol::parse(d.begin(), d.end())->as(msg_type(p));
}


template <
  typename Protocol,
  template <typename> typename AttributeProcessor
>
using attr_t = turner::attribute_type_t<
  typename Protocol::traits_t,
  0x01,
  AttributeProcessor<typename Protocol::traits_t>
>;


// uint32 {{{1


TYPED_TEST(attribute_processor, read_uint32)
{
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x04"          // Length
    "\x12\x34\x56\x78"  // Value
  );
  auto &msg = from_wire(TypeParam(), data);

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0x12345678U, value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_uint32_last_attribute)
{
  auto data = wire_data(TypeParam(),
    "\x00\x02"          // Type (not expected 0x1)
    "\x00\x00"          // Length

    "\x00\x01"          // Type (expected 0x1)
    "\x00\x04"          // Length
    "\x12\x34\x56\x78"  // Value
  );
  auto &msg = from_wire(TypeParam(), data);

  attr_t<TypeParam, turner::uint32_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0x12345678U, value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_uint32_attribute_not_found)
{
  auto data = wire_data(TypeParam(),
    "\00\x02"           // Type (not expected 0x1)
    "\00\x04"           // Length
    "\12\x34\56\x78"    // Value
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\00\x01"           // Type
    "\00\x08"           // Length (past message)
    "\12\x34\56\x78"    // Value
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\00\x01"           // Type
    "\00\x03"           // Length (!= sizeof(uint32_t))
    "\12\x34\56\x78"    // Value
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"  // Type
    "\x00\x03"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_string_last_attribute)
{
  auto data = wire_data(TypeParam(),
    "\x00\x02"  // Type (not expected 0x1)
    "\x00\x00"  // Length

    "\x00\x01"  // Type (expected 0x1)
    "\x00\x03"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

  attr_t<TypeParam, turner::string_attribute_processor_t> attr;
  std::error_code error;
  auto value = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", value);

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_string_attribute_not_found)
{
  auto data = wire_data(TypeParam(),
    "\x00\x02"  // Type (not expected 0x1)
    "\x00\x03"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"  // Type
    "\x00\x05"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01" // Type
    "\x00\x00" // Length
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"  // Type
    "\x00\x03"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  auto [ begin, end ] = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", std::string(begin, end));

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_array_last_attribute)
{
  auto data = wire_data(TypeParam(),
    "\x00\x02"  // Type (not expected 0x1)
    "\x00\x00"  // Length

    "\x00\x01"  // Type (expected 0x1)
    "\x00\x03"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

  attr_t<TypeParam, turner::array_attribute_processor_t> attr;
  std::error_code error;
  auto [ begin, end ] = msg.read(attr, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ("str", std::string(begin, end));

  EXPECT_NO_THROW(msg.read(attr));
}


TYPED_TEST(attribute_processor, read_array_attribute_not_found)
{
  auto data = wire_data(TypeParam(),
    "\x00\x02"  // Type (not expected 0x1)
    "\x00\x03"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"  // Type
    "\x00\x05"  // Length
    "str\0"     // Value + padding
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"  // Type
    "\x00\x00"  // Length
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x07"          // Length
    "\x00\x00\x03\x00"  // Value
    "Msg\xab"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x02"          // Type (not expected 0x1)
    "\x00\x00"          // Length

    "\x00\x01"          // Type (expected 0x1)
    "\x00\x07"          // Length
    "\x00\x00\x03\x00"  // Value
    "Msg\xab"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x02"          // Type (not expected 0x1)
    "\x00\x07"          // Length
    "\x00\x00\x03\x00"  // Value
    "Msg\xab"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x09"          // Length (past message)
    "\x00\x00\x03\x00"  // Value
    "Msg\xab"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"  // Type
    "\x00\x00"  // Length (need at least 4B)
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv4
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x14"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv6
    "\x05\x06\x07\x08"
    "\x09\x0a\x0b\x0c"
    "\x0d\x0e\x0f\x10"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x02"          // Type (not expected 0x1)
    "\x00\x00"          // Length

    "\x00\x01"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv4
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x02"          // Type (not expected 0x1)
    "\x00\x00"          // Length

    "\x00\x01"          // Type
    "\x00\x14"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv6
    "\x05\x06\x07\x08"
    "\x09\x0a\x0b\x0c"
    "\x0d\x0e\x0f\x10"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x02"          // Type (not expected 0x1)
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv4
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x09"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv4
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x15"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv6
    "\x05\x06\x07\x08"
    "\x09\x0a\x0b\x0c"
    "\x0d\x0e\x0f\x10"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x14"          // Length
    "\x00\x01"          // Value (claimed v4 but have v6)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv6
    "\x05\x06\x07\x08"
    "\x09\x0a\x0b\x0c"
    "\x0d\x0e\x0f\x10"
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x08"          // Length
    "\x00\x02"          // Value (claimed v6 but have v4)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv4
  );
  auto &msg = from_wire(TypeParam(), data);

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
  auto data = wire_data(TypeParam(),
    "\x00\x01"          // Type
    "\x00\x08"          // Length
    "\x00\x99"          // Value (unexpected family)
    "\x12\x34"          // Port
    "\x01\x02\x03\x04"  // IPv4
  );
  auto &msg = from_wire(TypeParam(), data);

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
