#include <turner/common.test.hpp>


namespace turner_test { namespace {


using turn = turner_test::fixture;


// CHANNEL-NUMBER {{{1


TEST_F(turn, read_channel_number)
{
  auto data = wire_data(TURN(),
    "\x00\x0c"  // Type
    "\x00\x04"  // Length
    "\x40\x01"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  auto value = msg.read(turner::turn::channel_number, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(0x4001, value);

  EXPECT_NO_THROW(
    msg.read(turner::turn::channel_number)
  );
}


TEST_F(turn, read_channel_number_last_attribute)
{
  auto data = wire_data(TURN(),
    "\x00\x21"  // Type (not expected)
    "\x00\x00"  // Length

    "\x00\x0c"  // Type
    "\x00\x04"  // Length
    "\x40\x01"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  auto value = msg.read(turner::turn::channel_number, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(0x4001, value);

  EXPECT_NO_THROW(
    msg.read(turner::turn::channel_number)
  );
}


TEST_F(turn, read_channel_number_attribute_not_found)
{
  auto data = wire_data(TURN(),
    "\x00\x21"  // Type
    "\x00\x04"  // Length
    "\x40\x01"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::channel_number, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::turn::channel_number),
    std::system_error
  );
}


TEST_F(turn, read_channel_number_unexpected_attribute_length)
{
  auto data = wire_data(TURN(),
    "\x00\x0c"  // Type
    "\x00\x03"  // Length
    "\x40\x01"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::channel_number, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::turn::channel_number),
    std::system_error
  );
}


TEST_F(turn, read_channel_number_too_small)
{
  auto data = wire_data(TURN(),
    "\x00\x0c"  // Type
    "\x00\x04"  // Length
    "\x30\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::channel_number, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);

  EXPECT_THROW(
    msg.read(turner::turn::channel_number),
    std::system_error
  );
}


TEST_F(turn, read_channel_number_too_big)
{
  auto data = wire_data(TURN(),
    "\x00\x0c"  // Type
    "\x00\x04"  // Length
    "\x80\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::channel_number, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);

  EXPECT_THROW(
    msg.read(turner::turn::channel_number),
    std::system_error
  );
}


TEST_F(turn, write_channel_number)
{
  std::array<uint8_t, TURN::traits_t::header_size + 8> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::turn::channel_number, 0x4001, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(TURN(), data);
  EXPECT_EQ(8, msg.length());
  EXPECT_EQ(0x4001, msg.read(turner::turn::channel_number));
}


TEST_F(turn, write_channel_number_not_enough_room)
{
  std::array<uint8_t, TURN::traits_t::header_size + 7> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(7, writer.available());

  writer.write(turner::turn::channel_number, 0x4001, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(7, writer.available());

  EXPECT_THROW(
    build(TURN(), data).write(turner::turn::channel_number, 0x4001),
    std::system_error
  );
}


TEST_F(turn, write_channel_number_too_small)
{
  std::array<uint8_t, TURN::traits_t::header_size + 8> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::turn::channel_number, 0x3999, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
  EXPECT_EQ(8, writer.available());

  EXPECT_THROW(
    build(TURN(), data).write(turner::turn::channel_number, 0x3999),
    std::system_error
  );
}


TEST_F(turn, write_channel_number_too_big)
{
  std::array<uint8_t, TURN::traits_t::header_size + 8> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::turn::channel_number, 0x8000, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
  EXPECT_EQ(8, writer.available());

  EXPECT_THROW(
    build(TURN(), data).write(turner::turn::channel_number, 0x8000),
    std::system_error
  );
}


// REQUESTED-ADDRESS-FAMILY {{{1


TEST_F(turn, read_requested_address_family)
{
  auto data = wire_data(TURN(),
    "\x00\x17"  // Type
    "\x00\x04"  // Length
    "\x01\x00"  // Value (v4)
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  auto value = msg.read(turner::turn::requested_address_family, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(turner::turn::address_family_t::v4, value);

  EXPECT_NO_THROW(
    msg.read(turner::turn::requested_address_family)
  );
}


TEST_F(turn, read_requested_address_family_last_attribute)
{
  auto data = wire_data(TURN(),
    "\x00\x21"  // Type (not expected)
    "\x00\x00"  // Length

    "\x00\x17"  // Type
    "\x00\x04"  // Length
    "\x01\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  auto value = msg.read(turner::turn::requested_address_family, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(turner::turn::address_family_t::v4, value);

  EXPECT_NO_THROW(
    msg.read(turner::turn::requested_address_family)
  );
}


TEST_F(turn, read_requested_address_family_attribute_not_found)
{
  auto data = wire_data(TURN(),
    "\x00\x21"  // Type
    "\x00\x04"  // Length
    "\x01\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::requested_address_family, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::turn::requested_address_family),
    std::system_error
  );
}


TEST_F(turn, read_requested_address_family_unexpected_attribute_length)
{
  auto data = wire_data(TURN(),
    "\x00\x17"  // Type
    "\x00\x03"  // Length
    "\x01\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::requested_address_family, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::turn::requested_address_family),
    std::system_error
  );
}


TEST_F(turn, write_requested_address_family)
{
  std::array<uint8_t, TURN::traits_t::header_size + 8> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::turn::requested_address_family,
    turner::turn::address_family_t::v4,
    error
  );
  EXPECT_TRUE(!error);
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(TURN(), data);
  EXPECT_EQ(8, msg.length());
  EXPECT_EQ(
    turner::turn::address_family_t::v4,
    msg.read(turner::turn::requested_address_family)
  );
}


TEST_F(turn, write_requested_address_family_not_enough_room)
{
  std::array<uint8_t, TURN::traits_t::header_size + 7> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(7, writer.available());

  writer.write(turner::turn::requested_address_family,
    turner::turn::address_family_t::v4,
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(7, writer.available());

  EXPECT_THROW(
    build(TURN(), data).write(
      turner::turn::requested_address_family,
      turner::turn::address_family_t::v4
    ),
    std::system_error
  );
}


// REQUESTED-TRANSPORT {{{1


TEST_F(turn, read_requested_transport)
{
  auto data = wire_data(TURN(),
    "\x00\x19"  // Type
    "\x00\x04"  // Length
    "\x11\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  auto value = msg.read(turner::turn::requested_transport, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(turner::turn::transport_protocol_t::udp, value);

  EXPECT_NO_THROW(
    msg.read(turner::turn::requested_transport)
  );
}


TEST_F(turn, read_requested_transport_last_attribute)
{
  auto data = wire_data(TURN(),
    "\x00\x21"  // Type (not expected)
    "\x00\x00"  // Length

    "\x00\x19"  // Type
    "\x00\x04"  // Length
    "\x11\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  auto value = msg.read(turner::turn::requested_transport, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(turner::turn::transport_protocol_t::udp, value);

  EXPECT_NO_THROW(
    msg.read(turner::turn::requested_transport)
  );
}


TEST_F(turn, read_requested_transport_attribute_not_found)
{
  auto data = wire_data(TURN(),
    "\x00\x21"  // Type
    "\x00\x04"  // Length
    "\x11\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::requested_transport, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::turn::requested_transport),
    std::system_error
  );
}


TEST_F(turn, read_requested_transport_unexpected_attribute_length)
{
  auto data = wire_data(TURN(),
    "\x00\x19"  // Type
    "\x00\x03"  // Length
    "\x11\x00"  // Value
    "\x00\x00"
  );
  auto &msg = parse(TURN(), data);

  std::error_code error;
  msg.read(turner::turn::requested_transport, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::turn::requested_transport),
    std::system_error
  );
}


TEST_F(turn, write_requested_transport)
{
  std::array<uint8_t, TURN::traits_t::header_size + 8> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::turn::requested_transport,
    turner::turn::transport_protocol_t::udp,
    error
  );
  EXPECT_TRUE(!error);
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(TURN(), data);
  EXPECT_EQ(8, msg.length());
  EXPECT_EQ(
    turner::turn::transport_protocol_t::udp,
    msg.read(turner::turn::requested_transport)
  );
}


TEST_F(turn, write_requested_transport_not_enough_room)
{
  std::array<uint8_t, TURN::traits_t::header_size + 7> data;

  std::error_code error;
  auto writer = build(TURN(), data);
  EXPECT_EQ(7, writer.available());

  writer.write(turner::turn::requested_transport,
    turner::turn::transport_protocol_t::udp,
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(7, writer.available());

  EXPECT_THROW(
    build(TURN(), data).write(
      turner::turn::requested_transport,
      turner::turn::transport_protocol_t::udp
    ),
    std::system_error
  );
}


// read_many {{{1


TEST_F(turn, read_many)
{
  auto data = wire_data(TURN(),
    "\x00\x06"          // Username (comprehension required)
    "\x00\x08"          // Length
    "username"          // Value

    "\x80\x22"          // Software (comprehension optional)
    "\x00\x08"          // Length
    "software"          // Value
  );
  auto &msg = parse(TURN(), data);

  std::string_view username, software;

  std::error_code error;
  uint16_t failed[4];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::username.value_ref(username),
    turner::turn::software.value_ref(software)
  );
  EXPECT_EQ(0U, failed_count);
  EXPECT_TRUE(!error);
  EXPECT_EQ("username", username);
  EXPECT_EQ("software", software);
}


TEST_F(turn, read_many_unknown_comprehension_required)
{
  auto data = wire_data(TURN(),
    "\x00\x06"          // username
    "\x00\x08"          // Length
    "username"          // Value

    "\x80\x22"          // software
    "\x00\x08"          // Length
    "software"          // Value
  );
  auto &msg = parse(TURN(), data);

  std::string_view software;

  std::error_code error;
  uint16_t failed[4];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::software.value_ref(software)
  );
  EXPECT_EQ(turner::errc::unknown_comprehension_required, error);
  ASSERT_EQ(1U, failed_count);
  EXPECT_EQ(turner::turn::username.type, failed[0]);
}


TEST_F(turn, read_many_unknown_comprehension_required_overflow)
{
  auto data = wire_data(TURN(),
    "\x00\x06"          // username
    "\x00\x08"          // Length
    "username"          // Value

    "\x00\x20"          // xor_mapped_address
    "\x00\x08"          // Length
    "\x00\x01"          // Value
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4

    "\x80\x22"          // software
    "\x00\x08"          // Length
    "software"          // Value
  );
  auto &msg = parse(TURN(), data);

  std::string_view software;

  std::error_code error;
  uint16_t failed[1];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::software.value_ref(software)
  );
  EXPECT_EQ(turner::errc::unknown_comprehension_required, error);
  ASSERT_EQ(1U, failed_count);
  EXPECT_EQ(turner::turn::username.type, failed[0]);
}


TEST_F(turn, read_many_missing)
{
  auto data = wire_data(TURN(),
    "\x80\x22"          // software
    "\x00\x08"          // Length
    "software"          // Value
  );
  auto &msg = parse(TURN(), data);

  std::string_view username;

  std::error_code error;
  uint16_t failed[4];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::username.value_ref(username)
  );
  EXPECT_EQ(turner::errc::attribute_not_found, error);
  ASSERT_EQ(1U, failed_count);
  EXPECT_EQ(turner::turn::username.type, failed[0]);
}


TEST_F(turn, read_many_missing_overflow)
{
  auto data = wire_data(TURN(),
    "\x00\x06"          // username
    "\x00\x08"          // Length
    "username"          // Value
  );
  auto &msg = parse(TURN(), data);

  std::string_view username, software;
  turner::turn::alternate_server_t::value_t alternate_server;

  std::error_code error;
  uint16_t failed[1];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::username.value_ref(username),
    turner::turn::software.value_ref(software),
    turner::turn::alternate_server.value_ref(alternate_server)
  );
  EXPECT_EQ(turner::errc::attribute_not_found, error);
  ASSERT_EQ(1U, failed_count);
  EXPECT_EQ(turner::turn::software.type, failed[0]);
}


TEST_F(turn, read_many_unexpected_attribute_value)
{
  auto data = wire_data(TURN(),
    "\x00\x20"          // xor_mapped_address
    "\x00\x08"          // Length
    "\x00\xff"          // Value (invalid)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(TURN(), data);

  turner::turn::xor_mapped_address_t::value_t xor_mapped_address;

  std::error_code error;
  uint16_t failed[4];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::xor_mapped_address.value_ref(xor_mapped_address)
  );
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
  EXPECT_EQ(0U, failed_count);
}


TEST_F(turn, read_many_insufficient_payload_data)
{
  auto data = wire_data(TURN(),
    "\x00\x20"          // XOR-MAPPED-ADDRESS
    "\x00\x08"          // Length
    "\x00\xff"          // Value (invalid)
    "\x33\x26"          // Port, but no address
  );
  auto &msg = parse(TURN(), data);

  turner::turn::xor_mapped_address_t::value_t xor_mapped_address;

  std::error_code error;
  uint16_t failed[4];
  auto failed_count = msg.read_many(error, failed,
    turner::turn::xor_mapped_address.value_ref(xor_mapped_address)
  );
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);
  EXPECT_EQ(0U, failed_count);
}


// }}}1


}} // namespace turner_test
