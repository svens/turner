#include <turner/common.test.hpp>


namespace turner_test { namespace {


using stun = turner_test::fixture;


__turner_inline_var const auto expected_address_v4 =
  sal::net::ip::make_address("1.2.3.4");

__turner_inline_var const auto expected_address_v6 =
  sal::net::ip::make_address("102:304:506:708:90a:b0c:d0e:f10");

__turner_inline_var constexpr const uint16_t expected_port = 0x1234;


TEST_F(stun, read_xor_address_v4)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::stun::xor_mapped_address));
}


TEST_F(stun, read_xor_address_v6)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x14"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv6
    "\x05\x07\x05\x0b"
    "\x0d\x0f\x0d\x0b"
    "\x05\x07\x05\x1b"
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::stun::xor_mapped_address));
}


TEST_F(stun, read_xor_address_v4_last_attribute)
{
  auto data = wire_data(STUN(),
    "\x00\x21"          // Type (not expected)
    "\x00\x00"          // Length

    "\x00\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::stun::xor_mapped_address));
}


TEST_F(stun, read_xor_address_v6_last_attribute)
{
  auto data = wire_data(STUN(),
    "\x00\x21"          // Type (not expected)
    "\x00\x00"          // Length

    "\x00\x20"          // Type
    "\x00\x14"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv6
    "\x05\x07\x05\x0b"
    "\x0d\x0f\x0d\x0b"
    "\x05\x07\x05\x1b"
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::stun::xor_mapped_address));
}


TEST_F(stun, read_xor_address_attribute_not_found)
{
  auto data = wire_data(STUN(),
    "\x00\x21"          // Type (not expected)
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::stun::xor_mapped_address),
    std::system_error
  );
}


TEST_F(stun, read_xor_address_v4_length_past_message_end)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x09"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::stun::xor_mapped_address),
    std::system_error
  );
}


TEST_F(stun, read_xor_address_v6_length_past_message_end)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x15"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv6
    "\x05\x07\x05\x0b"
    "\x0d\x0f\x0d\x0b"
    "\x05\x07\x05\x1b"
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::stun::xor_mapped_address),
    std::system_error
  );
}


TEST_F(stun, read_xor_address_v4_unexpected_attribute_length)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x14"          // Length
    "\x00\x01"          // Value (claimed v4 but have v6)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv6
    "\x05\x07\x05\x0b"
    "\x0d\x0f\x0d\x0b"
    "\x05\x07\x05\x1b"
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::stun::xor_mapped_address),
    std::system_error
  );
}


TEST_F(stun, read_xor_address_v6_unexpected_attribute_length)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x02"          // Value (claimed v6 but have v4)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::stun::xor_mapped_address),
    std::system_error
  );
}


TEST_F(stun, read_xor_address_unexpected_address_family)
{
  auto data = wire_data(STUN(),
    "\x00\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x99"          // Value (unexpected family)
    "\x33\x26"          // Port
    "\x20\x10\xa7\x46"  // IPv4
  );
  auto &msg = parse(STUN(), data);

  std::error_code error;
  msg.read(turner::stun::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_address_family, error);

  EXPECT_THROW(
    msg.read(turner::stun::xor_mapped_address),
    std::system_error
  );
}


TEST_F(stun, write_xor_address_v4)
{
  std::array<uint8_t, STUN::traits_t::header_size + 12> data;

  std::error_code error;
  auto writer = build(STUN(), data);
  EXPECT_EQ(12U, writer.available());

  writer.write(turner::stun::xor_mapped_address,
    {expected_address_v4, expected_port},
    error
  );
  EXPECT_TRUE(!error) << error;
  EXPECT_EQ(0U, writer.available());

  auto &msg = parse(STUN(), data);
  EXPECT_EQ(12U, msg.length());
  auto [ address, port ] = msg.read(turner::stun::xor_mapped_address);
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);
}


TEST_F(stun, write_xor_address_v6)
{
  std::array<uint8_t, STUN::traits_t::header_size + 24> data;

  std::error_code error;
  auto writer = build(STUN(), data);
  EXPECT_EQ(24, writer.available());

  writer.write(turner::stun::xor_mapped_address,
    {expected_address_v6, expected_port},
    error
  );
  EXPECT_TRUE(!error) << error;
  EXPECT_EQ(0U, writer.available());

  auto &msg = parse(STUN(), data);
  EXPECT_EQ(24, msg.length());
  auto [ address, port ] = msg.read(turner::stun::xor_mapped_address);
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);
}


TEST_F(stun, write_xor_address_v4_not_enough_room)
{
  std::array<uint8_t, STUN::traits_t::header_size + 11> data;

  std::error_code error;
  auto writer = build(STUN(), data);
  EXPECT_EQ(11, writer.available());

  writer.write(turner::stun::xor_mapped_address,
    {expected_address_v4, expected_port},
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(11, writer.available());

  EXPECT_THROW(
    build(STUN(), data).write(turner::stun::xor_mapped_address,
      {expected_address_v4, expected_port}
    ),
    std::system_error
  );
}


TEST_F(stun, write_xor_address_v6_not_enough_room)
{
  std::array<uint8_t, STUN::traits_t::header_size + 23> data;

  std::error_code error;
  auto writer = build(STUN(), data);
  EXPECT_EQ(23, writer.available());

  writer.write(turner::stun::xor_mapped_address,
    {expected_address_v6, expected_port},
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(23, writer.available());

  EXPECT_THROW(
    build(STUN(), data).write(turner::stun::xor_mapped_address,
      {expected_address_v6, expected_port}
    ),
    std::system_error
  );
}


TEST_F(stun, write_xor_address_unexpected_family)
{
  std::array<uint8_t, STUN::traits_t::header_size + 24> data;

  std::error_code error;
  auto writer = build(STUN(), data);
  EXPECT_EQ(24, writer.available());

  auto address = expected_address_v4;
  reinterpret_cast<uint8_t *>(&address)[0] = 99;

  writer.write(turner::stun::xor_mapped_address,
    {address, expected_port},
    error
  );

  EXPECT_EQ(turner::errc::unexpected_address_family, error);
  EXPECT_EQ(24, writer.available());

  EXPECT_THROW(
    build(STUN(), data).write(turner::stun::xor_mapped_address,
      {address, expected_port}
    ),
    std::system_error
  );
}


}} // namespace turner_test
