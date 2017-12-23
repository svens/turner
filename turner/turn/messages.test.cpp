#include <turner/common.test.hpp>


namespace turner_test { namespace {


using turn = turner_test::fixture;


TEST_F(turn, allocation)
{
  EXPECT_EQ((uint16_t)0x0003, turner::turn::allocation);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_request(
      turner::turn::allocation.type
    )
  );

  EXPECT_EQ((uint16_t)0x0103, turner::turn::allocation_success);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_success_response(
      turner::turn::allocation_success.type
    )
  );

  EXPECT_EQ((uint16_t)0x0113, turner::turn::allocation_error);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_error_response(
      turner::turn::allocation_error.type
    )
  );
}


TEST_F(turn, refresh)
{
  EXPECT_EQ((uint16_t)0x0004, turner::turn::refresh);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_request(
      turner::turn::refresh.type
    )
  );

  EXPECT_EQ((uint16_t)0x0104, turner::turn::refresh_success);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_success_response(
      turner::turn::refresh_success.type
    )
  );

  EXPECT_EQ((uint16_t)0x0114, turner::turn::refresh_error);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_error_response(
      turner::turn::refresh_error.type
    )
  );
}


TEST_F(turn, send_indication)
{
  EXPECT_EQ((uint16_t)0x0016, turner::turn::send_indication);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_indication(
      turner::turn::send_indication.type
    )
  );
}


TEST_F(turn, data_indication)
{
  EXPECT_EQ((uint16_t)0x0017, turner::turn::data_indication);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_indication(
      turner::turn::data_indication.type
    )
  );
}


TEST_F(turn, create_permission)
{
  EXPECT_EQ((uint16_t)0x0008, turner::turn::create_permission);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_request(
      turner::turn::create_permission.type
    )
  );

  EXPECT_EQ((uint16_t)0x0108, turner::turn::create_permission_success);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_success_response(
      turner::turn::create_permission_success.type
    )
  );

  EXPECT_EQ((uint16_t)0x0118, turner::turn::create_permission_error);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_error_response(
      turner::turn::create_permission_error.type
    )
  );
}


TEST_F(turn, channel_bind)
{
  EXPECT_EQ((uint16_t)0x0009, turner::turn::channel_bind);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_request(
      turner::turn::channel_bind.type
    )
  );

  EXPECT_EQ((uint16_t)0x0109, turner::turn::channel_bind_success);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_success_response(
      turner::turn::channel_bind_success.type
    )
  );

  EXPECT_EQ((uint16_t)0x0119, turner::turn::channel_bind_error);
  EXPECT_TRUE(
    turner::turn::protocol_traits_t::is_error_response(
      turner::turn::channel_bind_error.type
    )
  );
}


}} // namespace turner_test
