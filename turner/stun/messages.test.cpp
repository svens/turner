#include <turner/common.test.hpp>


namespace turner_test { namespace {


using stun = turner_test::fixture;


TEST_F(stun, binding)
{
  EXPECT_EQ((uint16_t)0x0001, turner::stun::binding);
  EXPECT_TRUE(
    turner::stun::protocol_traits_t::is_request(turner::stun::binding.type)
  );

  EXPECT_EQ((uint16_t)0x0101, turner::stun::binding_success);
  EXPECT_TRUE(
    turner::stun::protocol_traits_t::is_success_response(
      turner::stun::binding_success.type
    )
  );
}


//
// https://tools.ietf.org/html/rfc5769 tests
// Note: FINGERPRINT is not used in response samples
//

const std::array<uint8_t, 108> sample_request =
{{
  0x00, 0x01, 0x00, 0x58,       //    Request type and message length
  0x21, 0x12, 0xa4, 0x42,       //    Magic cookie
  0xb7, 0xe7, 0xa7, 0x01,       // >
  0xbc, 0x34, 0xd6, 0x86,       // >  Transaction ID
  0xfa, 0x87, 0xdf, 0xae,       // >
  0x80, 0x22, 0x00, 0x10,       //    SOFTWARE attribute header
  0x53, 0x54, 0x55, 0x4e,       // >
  0x20, 0x74, 0x65, 0x73,       // >  User-agent...
  0x74, 0x20, 0x63, 0x6c,       // >  ...name
  0x69, 0x65, 0x6e, 0x74,       // >
  0x00, 0x24, 0x00, 0x04,       //    PRIORITY attribute header
  0x6e, 0x00, 0x01, 0xff,       //    ICE priority value
  0x80, 0x29, 0x00, 0x08,       //    ICE-CONTROLLED attribute header
  0x93, 0x2f, 0xf9, 0xb1,       // >  Pseudo-random tie breaker...
  0x51, 0x26, 0x3b, 0x36,       // >   ...for ICE control
  0x00, 0x06, 0x00, 0x09,       //    USERNAME attribute header
  0x65, 0x76, 0x74, 0x6a,       // >
  0x3a, 0x68, 0x36, 0x76,       // >  Username (9 bytes) and padding (3 bytes)
  0x59, 0x20, 0x20, 0x20,       // >
  0x00, 0x08, 0x00, 0x14,       //    MESSAGE-INTEGRITY attribute header
  0x9a, 0xea, 0xa7, 0x0c,       // >
  0xbf, 0xd8, 0xcb, 0x56,       // >
  0x78, 0x1e, 0xf2, 0xb5,       // >  HMAC-SHA1 fingerprint
  0xb2, 0xd3, 0xf2, 0x49,       // >
  0xc1, 0xb5, 0x71, 0xa2,       // >
  0x80, 0x28, 0x00, 0x04,       //    FINGERPRINT attribute header
  0xe5, 0x7a, 0x3b, 0xcf,       //    CRC32 fingerprint
}};


TEST_F(stun, rfc5769_sample_request)
{
  auto msg = turner::stun::protocol.parse(
    sample_request.begin(), sample_request.end()
  );

  auto &binding = msg->as(turner::stun::binding);
  EXPECT_EQ("STUN test client", binding.read(turner::stun::software));
  EXPECT_EQ("evtj:h6vY", binding.read(turner::stun::username));

  auto integrity_calculator = turner::stun::make_integrity_calculator(
    "VOkJxbRl1RmTxUk/WvJxBt"
  );
  EXPECT_TRUE(binding.has_valid_integrity(integrity_calculator));
}


TEST_F(stun, rfc5769_sample_ipv4_response)
{
  // FINGERPRINT is stripped
  const std::array<uint8_t, 72> expected =
  {{
    0x01, 0x01, 0x00, 0x34,     //    Response type and message length
    0x21, 0x12, 0xa4, 0x42,     //    Magic cookie
    0xb7, 0xe7, 0xa7, 0x01,     // >
    0xbc, 0x34, 0xd6, 0x86,     // >  Transaction ID
    0xfa, 0x87, 0xdf, 0xae,     // >
    0x80, 0x22, 0x00, 0x0b,     //    SOFTWARE attribute header
    0x74, 0x65, 0x73, 0x74,     // >
    0x20, 0x76, 0x65, 0x63,     // >  UTF-8 server name
    0x74, 0x6f, 0x72, 0x20,     // >
    0x00, 0x20, 0x00, 0x08,     //    XOR-MAPPED-ADDRESS attribute header
    0x00, 0x01, 0xa1, 0x47,     //    Address family (IPv4) and xor'd mapped port number
    0xe1, 0x12, 0xa6, 0x43,     //    Xor'd mapped IPv4 address
    0x00, 0x08, 0x00, 0x14,     //    MESSAGE-INTEGRITY attribute header
    0x2b, 0x91, 0xf5, 0x99,     // >
    0xfd, 0x9e, 0x90, 0xc3,     // >
    0x8c, 0x74, 0x89, 0xf9,     // >  HMAC-SHA1 fingerprint
    0x2a, 0xf9, 0xba, 0x53,     // >
    0xf0, 0x6b, 0xe7, 0xd7,     // >
  }};

  // sample_request
  auto &binding = turner::stun::protocol.parse(
    sample_request.begin(), sample_request.end()
  )->as(turner::stun::binding);

  // destination area (padded with spaces)
  std::array<uint8_t, expected.size()> data;
  std::fill(data.begin(), data.end(), ' ');

  // generate response
  auto response = binding.to_success_response(data);
  response.write(turner::stun::software, "test vector");
  response.write(turner::stun::xor_mapped_address,
    {
      sal::net::ip::make_address_v4("192.0.2.1"),
      static_cast<uint16_t>(32853)
    }
  );

  // finalize
  auto integrity_calculator = turner::stun::make_integrity_calculator(
    "VOkJxbRl1RmTxUk/WvJxBt"
  );
  auto [ begin, end ] = response.finish(integrity_calculator);
  EXPECT_EQ(data.data(), begin);
  EXPECT_EQ(data.data() + data.size(), end);
  EXPECT_EQ(expected, data);
}


TEST_F(stun, rfc5769_sample_ipv6_response)
{
  // FINGERPRINT is stripped
  const std::array<uint8_t, 84> expected =
  {{
    0x01, 0x01, 0x00, 0x40,     //    Response type and message length
    0x21, 0x12, 0xa4, 0x42,     //    Magic cookie
    0xb7, 0xe7, 0xa7, 0x01,     // >
    0xbc, 0x34, 0xd6, 0x86,     // >  Transaction ID
    0xfa, 0x87, 0xdf, 0xae,     // >
    0x80, 0x22, 0x00, 0x0b,     //    SOFTWARE attribute header
    0x74, 0x65, 0x73, 0x74,     // >
    0x20, 0x76, 0x65, 0x63,     // >  UTF-8 server name
    0x74, 0x6f, 0x72, 0x20,     // >
    0x00, 0x20, 0x00, 0x14,     //    XOR-MAPPED-ADDRESS attribute header
    0x00, 0x02, 0xa1, 0x47,     //    Address family (IPv6) and xor'd mapped port number
    0x01, 0x13, 0xa9, 0xfa,     // >
    0xa5, 0xd3, 0xf1, 0x79,     // >  Xor'd mapped IPv6 address
    0xbc, 0x25, 0xf4, 0xb5,     // >
    0xbe, 0xd2, 0xb9, 0xd9,     // >
    0x00, 0x08, 0x00, 0x14,     //    MESSAGE-INTEGRITY attribute header
    0xa3, 0x82, 0x95, 0x4e,     // >
    0x4b, 0xe6, 0x7b, 0xf1,     // >
    0x17, 0x84, 0xc9, 0x7c,     // >  HMAC-SHA1 fingerprint
    0x82, 0x92, 0xc2, 0x75,     // >
    0xbf, 0xe3, 0xed, 0x41,     // >
  }};

  // sample_request
  auto &binding = turner::stun::protocol.parse(
    sample_request.begin(), sample_request.end()
  )->as(turner::stun::binding);

  // destination area (padded with spaces)
  std::array<uint8_t, expected.size()> data;
  std::fill(data.begin(), data.end(), ' ');

  // generate response
  auto response = binding.to_success_response(data);
  response.write(turner::stun::software, "test vector");
  response.write(turner::stun::xor_mapped_address,
    {
      sal::net::ip::make_address_v6("2001:db8:1234:5678:11:2233:4455:6677"),
      static_cast<uint16_t>(32853)
    }
  );

  // finalize
  auto integrity_calculator = turner::stun::make_integrity_calculator(
    "VOkJxbRl1RmTxUk/WvJxBt"
  );
  auto [ begin, end ] = response.finish(integrity_calculator);
  EXPECT_EQ(data.data(), begin);
  EXPECT_EQ(data.data() + data.size(), end);
  EXPECT_EQ(expected, data);
}


TEST_F(stun, rfc5769_sample_request_with_long_term_authentication)
{
  // FINGERPRINT is stripped
  const std::array<uint8_t, 116> expected =
  {{
    0x00, 0x01, 0x00, 0x60,     //     Request type and message length
    0x21, 0x12, 0xa4, 0x42,     //     Magic cookie
    0x78, 0xad, 0x34, 0x33,     //  >
    0xc6, 0xad, 0x72, 0xc0,     //  >  Transaction ID
    0x29, 0xda, 0x41, 0x2e,     //  >
    0x00, 0x06, 0x00, 0x12,     //     USERNAME attribute header
    0xe3, 0x83, 0x9e, 0xe3,     //  >
    0x83, 0x88, 0xe3, 0x83,     //  >
    0xaa, 0xe3, 0x83, 0x83,     //  >  Username value (18 bytes) and padding (2 bytes)
    0xe3, 0x82, 0xaf, 0xe3,     //  >
    0x82, 0xb9, 0x00, 0x00,     //  >
    0x00, 0x15, 0x00, 0x1c,     //     NONCE attribute header
    0x66, 0x2f, 0x2f, 0x34,     //  >
    0x39, 0x39, 0x6b, 0x39,     //  >
    0x35, 0x34, 0x64, 0x36,     //  >
    0x4f, 0x4c, 0x33, 0x34,     //  >  Nonce value
    0x6f, 0x4c, 0x39, 0x46,     //  >
    0x53, 0x54, 0x76, 0x79,     //  >
    0x36, 0x34, 0x73, 0x41,     //  >
    0x00, 0x14, 0x00, 0x0b,     //     REALM attribute header
    0x65, 0x78, 0x61, 0x6d,     //  >
    0x70, 0x6c, 0x65, 0x2e,     //  >  Realm value (11 bytes) and padding (1 byte)
    0x6f, 0x72, 0x67, 0x00,     //  >
    0x00, 0x08, 0x00, 0x14,     //     MESSAGE-INTEGRITY attribute header
    0xf6, 0x70, 0x24, 0x65,     //  >
    0x6d, 0xd6, 0x4a, 0x3e,     //  >
    0x02, 0xb8, 0xe0, 0x71,     //  >  HMAC-SHA1 fingerprint
    0x2e, 0x85, 0xc9, 0xa2,     //  >
    0x8c, 0xa8, 0x96, 0x66,     //  >
  }};

  //
  // pretend building message
  //

  // instantiate writer with data area
  std::array<uint8_t, expected.max_size()> data;
  std::error_code error;
  auto writer = turner::stun::protocol.build(turner::stun::binding,
    data.begin(), data.end()
  );

  // instead of building message, copy expected without MESSAGE-INTEGRITY
  // and update length accordingly
  std::uninitialized_copy_n(expected.begin(),
    expected.size() - 24,
    data.begin()
  );
  data[3] -= 24;
  EXPECT_EQ(
    expected.size() - turner::stun::protocol_traits_t::header_size - 24,
    writer.length()
  );

  // actual testing part: finish with integrity
  auto integrity_calculator = turner::stun::make_integrity_calculator(
    "example.org",
    "\xe3\x83\x9e\xe3\x83\x88\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9",
    "TheMatrIX"
  );
  auto [ begin, end ] = writer.finish(integrity_calculator);
  EXPECT_EQ(data.data(), begin);
  EXPECT_EQ(data.data() + data.size(), end);
  EXPECT_EQ(expected, data);
}


}} // namespace turner_test
