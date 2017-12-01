#include <turner/common.test.hpp>


namespace turner_test { namespace {


using stun = turner_test::fixture;


TEST_F(stun, binding_name)
{
  EXPECT_STREQ("binding", turner::stun::binding.name());
  EXPECT_STREQ("binding_success", turner::stun::binding_success.name());
}


TEST_F(stun, rfc5769_sample_request_with_long_term_authentication)
{
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
