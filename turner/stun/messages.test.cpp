#include <turner/common.test.hpp>


namespace turner_test { namespace {


using stun = turner_test::fixture;


TEST_F(stun, binding_name)
{
  EXPECT_STREQ("binding", turner::stun::binding.name());
  EXPECT_STREQ("binding_success", turner::stun::binding_success.name());
}


TEST_F(stun, make_integrity_calculator)
{
  // Stripped example from https://tools.ietf.org/html/rfc5769#section-2.4
  // (no MESSAGE-INTEGRITY appended)
  const char msg[] =
    "\x00\x01\x00\x60"  //     Request type and message length
    "\x21\x12\xa4\x42"  //     Magic cookie
    "\x78\xad\x34\x33"  //  }
    "\xc6\xad\x72\xc0"  //  }  Transaction ID
    "\x29\xda\x41\x2e"  //  }
    "\x00\x06\x00\x12"  //     USERNAME attribute header
    "\xe3\x83\x9e\xe3"  //  }
    "\x83\x88\xe3\x83"  //  }
    "\xaa\xe3\x83\x83"  //  }  Username value (18 bytes) and padding (2 bytes)
    "\xe3\x82\xaf\xe3"  //  }
    "\x82\xb9\x00\x00"  //  }
    "\x00\x15\x00\x1c"  //     NONCE attribute header
    "\x66\x2f\x2f\x34"  //  }
    "\x39\x39\x6b\x39"  //  }
    "\x35\x34\x64\x36"  //  }
    "\x4f\x4c\x33\x34"  //  }  Nonce value
    "\x6f\x4c\x39\x46"  //  }
    "\x53\x54\x76\x79"  //  }
    "\x36\x34\x73\x41"  //  }
    "\x00\x14\x00\x0b"  //     REALM attribute header
    "\x65\x78\x61\x6d"  //  }
    "\x70\x6c\x65\x2e"  //  }  Realm value (11 bytes) and padding (1 byte)
    "\x6f\x72\x67\x00"  //  }
  ;

  auto integrity_calculator = turner::stun::make_integrity_calculator(
    "example.org",
    "\xe3\x83\x9e\xe3\x83\x88\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9",
    "TheMatrIX"
  );
  integrity_calculator.update(msg, msg + sizeof(msg) - 1);
  auto integrity = integrity_calculator.finish();

  std::array<uint8_t, 20> expected_integrity =
  {{
    0xf6, 0x70, 0x24, 0x65,
    0x6d, 0xd6, 0x4a, 0x3e,
    0x02, 0xb8, 0xe0, 0x71,
    0x2e, 0x85, 0xc9, 0xa2,
    0x8c, 0xa8, 0x96, 0x66,
  }};

  EXPECT_EQ(expected_integrity, integrity);
}


}} // namespace turner_test
