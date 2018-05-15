#include <turner/common.test.hpp>


namespace turner_test { namespace {


using msturn = turner_test::fixture;


// MS-Version {{{1


TEST_F(msturn, read_ms_version)
{
  auto data = wire_data(MSTURN(),
    "\x80\x08"  // Type
    "\x00\x04"  // Length
    "\x00\x00"  // Value
    "\x00\x03"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::ms_version, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(turner::msturn::protocol_version_t::sha256, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::ms_version)
  );
}


TEST_F(msturn, read_ms_version_last_attribute)
{
  auto data = wire_data(MSTURN(),
    "\x00\x09"  // Type (not expected)
    "\x00\x00"  // Length

    "\x80\x08"  // Type
    "\x00\x04"  // Length
    "\x00\x00"  // Value
    "\x00\x03"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::ms_version, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(turner::msturn::protocol_version_t::sha256, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::ms_version)
  );
}


TEST_F(msturn, read_ms_version_attribute_not_found)
{
  auto data = wire_data(MSTURN(),
    "\x80\x09"  // Type
    "\x00\x04"  // Length
    "\x00\x00"  // Value
    "\x00\x03"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_version, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_version),
    std::system_error
  );
}


TEST_F(msturn, read_ms_version_unexpected_attribute_length)
{
  auto data = wire_data(MSTURN(),
    "\x80\x08"  // Type
    "\x00\x03"  // Length
    "\x00\x00"  // Value
    "\x00\x03"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_version, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_version),
    std::system_error
  );
}


TEST_F(msturn, write_ms_version)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 8> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::msturn::ms_version,
    turner::msturn::protocol_version_t::sha256,
    error
  );
  EXPECT_TRUE(!error);
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(MSTURN(), data);
  EXPECT_EQ(8 + MSTURN::min_payload_length(), msg.length());
  EXPECT_EQ(
    turner::msturn::protocol_version_t::sha256,
    msg.read(turner::msturn::ms_version)
  );
}


TEST_F(msturn, write_ms_version_not_enough_room)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 7> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(7, writer.available());

  writer.write(turner::msturn::ms_version,
    turner::msturn::protocol_version_t::sha256,
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(7, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data).write(
      turner::msturn::ms_version,
      turner::msturn::protocol_version_t::sha256
    ),
    std::system_error
  );
}


// MS-Sequence Number {{{1


constexpr auto sequence_number = std::make_pair(
  std::array<uint8_t, 20>
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
  }},
  0x01020304U
);


TEST_F(msturn, read_ms_sequence_number)
{
  auto data = wire_data(MSTURN(),
    "\x80\x50"          // Type
    "\x00\x18"          // Length
    "\x00\x01\x02\x03"  // Value (Connection ID)
    "\x04\x05\x06\x07"
    "\x08\x09\x0a\x0b"
    "\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13"
    "\x01\x02\x03\x04"  // (Sequence Number)
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::ms_sequence_number, error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(sequence_number, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::ms_sequence_number)
  );
}


TEST_F(msturn, read_ms_sequence_number_last_attribute)
{
  auto data = wire_data(MSTURN(),
    "\x00\x09"          // Type (not expected)
    "\x00\x00"          // Length

    "\x80\x50"          // Type
    "\x00\x18"          // Length
    "\x00\x01\x02\x03"  // Value (Connection ID)
    "\x04\x05\x06\x07"
    "\x08\x09\x0a\x0b"
    "\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13"
    "\x01\x02\x03\x04"  // (Sequence Number)
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::ms_sequence_number, error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(sequence_number, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::ms_sequence_number)
  );
}


TEST_F(msturn, read_ms_sequence_number_attribute_not_found)
{
  auto data = wire_data(MSTURN(),
    "\x80\x51"          // Type
    "\x00\x18"          // Length
    "\x00\x01\x02\x03"  // Value (Connection ID)
    "\x04\x05\x06\x07"
    "\x08\x09\x0a\x0b"
    "\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13"
    "\x01\x02\x03\x04"  // (Sequence Number)
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_sequence_number, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_sequence_number),
    std::system_error
  );
}


TEST_F(msturn, read_ms_sequence_number_unexpected_attribute_length)
{
  auto data = wire_data(MSTURN(),
    "\x80\x50"          // Type
    "\x00\x14"          // Length (invalid)
    "\x00\x01\x02\x03"  // Value (Connection ID)
    "\x04\x05\x06\x07"
    "\x08\x09\x0a\x0b"
    "\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13"
    "\x01\x02\x03\x04"  // (Sequence Number)
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_sequence_number, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_sequence_number),
    std::system_error
  );
}


TEST_F(msturn, read_ms_sequence_number_length_past_message_end)
{
  auto data = wire_data(MSTURN(),
    "\x80\x50"          // Type
    "\x00\x18"          // Length (invalid)
    "\x00\x01\x02\x03"  // Value (Connection ID)
    "\x04\x05\x06\x07"
    "\x08\x09\x0a\x0b"
    "\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13"
    // missing sequence number
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_sequence_number, error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_sequence_number),
    std::system_error
  );
}


TEST_F(msturn, write_ms_sequence_number)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 28> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(28, writer.available());

  writer.write(turner::msturn::ms_sequence_number, sequence_number, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(MSTURN(), data);
  EXPECT_EQ(28 + MSTURN::min_payload_length(), msg.length());
  EXPECT_EQ(sequence_number, msg.read(turner::msturn::ms_sequence_number));
}


TEST_F(msturn, write_ms_sequence_number_not_enough_room)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 27> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(27, writer.available());

  writer.write(turner::msturn::ms_sequence_number, sequence_number, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(27, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data).write(turner::msturn::ms_sequence_number, sequence_number),
    std::system_error
  );
}


// XOR Mapped Address {{{1


inline const auto expected_address_v4 = sal::net::ip::make_address("1.2.3.4");

inline const auto expected_address_v6 =
  sal::net::ip::make_address("102:304:506:708:90a:b0c:d0e:f10");

constexpr uint16_t expected_port = 0x1234;


TEST_F(msturn, read_xor_address_v4)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv4
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::msturn::xor_mapped_address));
}


TEST_F(msturn, read_xor_address_v6)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x14"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv6
    "\x01\x03\x01\x0f"
    "\x01\x03\x01\x07"
    "\x01\x03\x01\x1f"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::msturn::xor_mapped_address));
}


TEST_F(msturn, read_xor_address_v4_last_attribute)
{
  auto data = wire_data(MSTURN(),
    "\x00\x21"          // Type (not expected)
    "\x00\x00"          // Length

    "\x80\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv4
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::msturn::xor_mapped_address));
}


TEST_F(msturn, read_xor_address_v6_last_attribute)
{
  auto data = wire_data(MSTURN(),
    "\x00\x21"          // Type (not expected)
    "\x00\x00"          // Length

    "\x80\x20"          // Type
    "\x00\x14"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv6
    "\x01\x03\x01\x0f"
    "\x01\x03\x01\x07"
    "\x01\x03\x01\x1f"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto [ address, port ] = msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_TRUE(!error) << error.message();
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);

  EXPECT_NO_THROW(msg.read(turner::msturn::xor_mapped_address));
}


TEST_F(msturn, read_xor_address_attribute_not_found)
{
  auto data = wire_data(MSTURN(),
    "\x80\x21"          // Type
    "\x00\x08"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv4
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::msturn::xor_mapped_address),
    std::system_error
  );
}


TEST_F(msturn, read_xor_address_v4_length_past_message_end)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x09"          // Length
    "\x00\x01"          // Value (v4 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv4
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);

  EXPECT_THROW(
    msg.read(turner::msturn::xor_mapped_address),
    std::system_error
  );
}


TEST_F(msturn, read_xor_address_v6_length_past_message_end)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x15"          // Length
    "\x00\x02"          // Value (v6 address)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv6
    "\x01\x03\x01\x0f"
    "\x01\x03\x01\x07"
    "\x01\x03\x01\x1f"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);

  EXPECT_THROW(
    msg.read(turner::msturn::xor_mapped_address),
    std::system_error
  );
}


TEST_F(msturn, read_xor_address_v4_unexpected_attribute_length)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x14"          // Length
    "\x00\x01"          // Value (claimed v4 but have v6)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv6
    "\x01\x03\x01\x0f"
    "\x01\x03\x01\x07"
    "\x01\x03\x01\x1f"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::msturn::xor_mapped_address),
    std::system_error
  );
}


TEST_F(msturn, read_xor_address_v6_unexpected_attribute_length)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x02"          // Value (claimed v6 but have v4)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv4
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::msturn::xor_mapped_address),
    std::system_error
  );
}


TEST_F(msturn, read_xor_address_unexpected_attribute_value)
{
  auto data = wire_data(MSTURN(),
    "\x80\x20"          // Type
    "\x00\x08"          // Length
    "\x00\x99"          // Value (unexpected family)
    "\x12\x35"          // Port
    "\x01\x03\x01\x07"  // IPv4
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::xor_mapped_address, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);

  EXPECT_THROW(
    msg.read(turner::msturn::xor_mapped_address),
    std::system_error
  );
}


TEST_F(msturn, write_xor_address_v4)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 12> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(12U, writer.available());

  writer.write(turner::msturn::xor_mapped_address,
    {expected_address_v4, expected_port},
    error
  );
  EXPECT_TRUE(!error) << error;
  EXPECT_EQ(0U, writer.available());

  auto &msg = parse(MSTURN(), data);
  EXPECT_EQ(12U + MSTURN::min_payload_length(), msg.length());
  auto [ address, port ] = msg.read(turner::msturn::xor_mapped_address);
  EXPECT_EQ(expected_address_v4, address);
  EXPECT_EQ(expected_port, port);
}


TEST_F(msturn, write_xor_address_v6)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 24> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(24, writer.available());

  writer.write(turner::msturn::xor_mapped_address,
    {expected_address_v6, expected_port},
    error
  );
  EXPECT_TRUE(!error) << error;
  EXPECT_EQ(0U, writer.available());

  auto &msg = parse(MSTURN(), data);
  EXPECT_EQ(24 + MSTURN::min_payload_length(), msg.length());
  auto [ address, port ] = msg.read(turner::msturn::xor_mapped_address);
  EXPECT_EQ(expected_address_v6, address);
  EXPECT_EQ(expected_port, port);
}


TEST_F(msturn, write_xor_address_v4_not_enough_room)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 11> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(11, writer.available());

  writer.write(turner::msturn::xor_mapped_address,
    {expected_address_v4, expected_port},
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(11, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data).write(turner::msturn::xor_mapped_address,
      {expected_address_v4, expected_port}
    ),
    std::system_error
  );
}


TEST_F(msturn, write_xor_address_v6_not_enough_room)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 23> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(23, writer.available());

  writer.write(turner::msturn::xor_mapped_address,
    {expected_address_v6, expected_port},
    error
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(23, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data).write(turner::msturn::xor_mapped_address,
      {expected_address_v6, expected_port}
    ),
    std::system_error
  );
}


TEST_F(msturn, write_xor_address_unexpected_attribute_value)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 24> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(24, writer.available());

  auto address = expected_address_v4;
  reinterpret_cast<uint8_t *>(&address)[0] = 99;

  writer.write(turner::msturn::xor_mapped_address,
    {address, expected_port},
    error
  );

  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
  EXPECT_EQ(24, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data).write(turner::msturn::xor_mapped_address,
      {address, expected_port}
    ),
    std::system_error
  );
}


// MS-Service Quality {{{1


constexpr auto service_quality =
  std::make_pair(
    turner::msturn::stream_type_t::data,
    turner::msturn::service_quality_t::reliable_delivery
  );


TEST_F(msturn, read_ms_service_quality)
{
  auto data = wire_data(MSTURN(),
    "\x80\x55"  // Type
    "\x00\x04"  // Length
    "\x00\x04"  // Value
    "\x00\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::ms_service_quality, error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(service_quality, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::ms_service_quality)
  );
}


TEST_F(msturn, read_ms_service_quality_last_attribute)
{
  auto data = wire_data(MSTURN(),
    "\x00\x21"  // Type (not expected)
    "\x00\x00"  // Length

    "\x80\x55"  // Type
    "\x00\x04"  // Length
    "\x00\x04"  // Value
    "\x00\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::ms_service_quality, error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(service_quality, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::ms_service_quality)
  );
}


TEST_F(msturn, read_ms_service_quality_attribute_not_found)
{
  auto data = wire_data(MSTURN(),
    "\x80\x56"  // Type
    "\x00\x04"  // Length
    "\x00\x04"  // Value
    "\x00\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_service_quality, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_service_quality),
    std::system_error
  );
}


TEST_F(msturn, read_ms_service_quality_unexpected_attribute_length)
{
  auto data = wire_data(MSTURN(),
    "\x80\x55"  // Type
    "\x00\x03"  // Length
    "\x00\x04"  // Value
    "\x00\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_service_quality, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_service_quality),
    std::system_error
  );
}


TEST_F(msturn, read_ms_service_quality_unexpected_stream_type)
{
  auto data = wire_data(MSTURN(),
    "\x80\x55"  // Type
    "\x00\x04"  // Length
    "\x00\x00"  // Value
    "\x00\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_service_quality, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_service_quality),
    std::system_error
  );
}


TEST_F(msturn, read_ms_service_quality_unexpected_service_quality)
{
  auto data = wire_data(MSTURN(),
    "\x80\x55"  // Type
    "\x00\x04"  // Length
    "\x00\x04"  // Value
    "\x00\xff"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::ms_service_quality, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);

  EXPECT_THROW(
    msg.read(turner::msturn::ms_service_quality),
    std::system_error
  );
}


TEST_F(msturn, write_ms_service_quality)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 8> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(8, writer.available());

  writer.write(turner::msturn::ms_service_quality, service_quality, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(MSTURN(), data);
  EXPECT_EQ(8 + MSTURN::min_payload_length(), msg.length());
  EXPECT_EQ(service_quality, msg.read(turner::msturn::ms_service_quality));
}


TEST_F(msturn, write_ms_service_quality_not_enough_room)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 7> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(7, writer.available());

  writer.write(turner::msturn::ms_service_quality, service_quality, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(7, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data)
      .write(turner::msturn::ms_service_quality, service_quality),
    std::system_error
  );
}


// Multiplexed TURN Session ID {{{1


constexpr uint64_t session_id = 0x0123456789abcdef;


TEST_F(msturn, read_mturn_session_id)
{
  auto data = wire_data(MSTURN(),
    "\x80\x95"          // Type
    "\x00\x08"          // Length
    "\xef\xcd\xab\x89"  // Value
    "\x67\x45\x23\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::mturn_session_id, error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(session_id, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::mturn_session_id)
  );
}


TEST_F(msturn, read_mturn_session_id_last_attribute)
{
  auto data = wire_data(MSTURN(),
    "\x00\x09"          // Type (not expected)
    "\x00\x00"          // Length

    "\x80\x95"          // Type
    "\x00\x08"          // Length
    "\xef\xcd\xab\x89"  // Value
    "\x67\x45\x23\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  auto value = msg.read(turner::msturn::mturn_session_id, error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(session_id, value);

  EXPECT_NO_THROW(
    msg.read(turner::msturn::mturn_session_id)
  );
}


TEST_F(msturn, read_mturn_session_id_attribute_not_found)
{
  auto data = wire_data(MSTURN(),
    "\x80\x96"          // Type (not expected)
    "\x00\x08"          // Length
    "\xef\xcd\xab\x89"  // Value
    "\x67\x45\x23\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::mturn_session_id, error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);

  EXPECT_THROW(
    msg.read(turner::msturn::mturn_session_id),
    std::system_error
  );
}


TEST_F(msturn, read_mturn_session_id_unexpected_attribute_length)
{
  auto data = wire_data(MSTURN(),
    "\x80\x95"          // Type
    "\x00\x07"          // Length (invalid)
    "\xef\xcd\xab\x89"  // Value
    "\x67\x45\x23\x01"
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::mturn_session_id, error);
  EXPECT_EQ(turner::errc::unexpected_attribute_length, error);

  EXPECT_THROW(
    msg.read(turner::msturn::mturn_session_id),
    std::system_error
  );
}


TEST_F(msturn, read_mturn_session_id_length_past_message_end)
{
  auto data = wire_data(MSTURN(),
    "\x80\x95"          // Type
    "\x00\x08"          // Length
    "\xef\xcd\xab\x89"  // Value
  );
  auto &msg = parse(MSTURN(), data);

  std::error_code error;
  msg.read(turner::msturn::mturn_session_id, error);
  EXPECT_EQ(turner::errc::insufficient_payload_data, error);

  EXPECT_THROW(
    msg.read(turner::msturn::mturn_session_id),
    std::system_error
  );
}


TEST_F(msturn, write_mturn_session_id)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 12> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(12, writer.available());

  writer.write(turner::msturn::mturn_session_id, session_id, error);
  EXPECT_TRUE(!error);
  EXPECT_EQ(0, writer.available());

  auto &msg = parse(MSTURN(), data);
  EXPECT_EQ(12 + MSTURN::min_payload_length(), msg.length());
  EXPECT_EQ(session_id, msg.read(turner::msturn::mturn_session_id));
}


TEST_F(msturn, write_mturn_session_id_not_enough_room)
{
  std::array<uint8_t, MSTURN::header_and_cookie_size() + 11> data;

  std::error_code error;
  auto writer = build(MSTURN(), data);
  EXPECT_EQ(11, writer.available());

  writer.write(turner::msturn::mturn_session_id, session_id, error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(11, writer.available());

  EXPECT_THROW(
    build(MSTURN(), data).write(turner::msturn::mturn_session_id, session_id),
    std::system_error
  );
}


// }}}1


}} // namespace turner_test
