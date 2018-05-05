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


// }}}1


}} // namespace turner_test
