#include <turner/common.test.hpp>


namespace turner_test { namespace {


using turn = turner_test::fixture;


TEST_F(turn, allocaton)
{
  EXPECT_EQ(0x0003, turner::turn::allocation.type());
  EXPECT_TRUE(turner::turn::allocation.is_request());

  EXPECT_EQ(0x0103, turner::turn::allocation_success.type());
  EXPECT_TRUE(turner::turn::allocation_success.is_success_response());

  EXPECT_EQ(0x0113, turner::turn::allocation_error.type());
  EXPECT_TRUE(turner::turn::allocation_error.is_error_response());
}


TEST_F(turn, refresh)
{
  EXPECT_EQ(0x0004, turner::turn::refresh.type());
  EXPECT_TRUE(turner::turn::refresh.is_request());

  EXPECT_EQ(0x0104, turner::turn::refresh_success.type());
  EXPECT_TRUE(turner::turn::refresh_success.is_success_response());

  EXPECT_EQ(0x0114, turner::turn::refresh_error.type());
  EXPECT_TRUE(turner::turn::refresh_error.is_error_response());
}


TEST_F(turn, send_indication)
{
  EXPECT_EQ(0x0016, turner::turn::send_indication.type());
  EXPECT_TRUE(turner::turn::send_indication.is_indication());
}


TEST_F(turn, data_indication)
{
  EXPECT_EQ(0x0017, turner::turn::data_indication.type());
  EXPECT_TRUE(turner::turn::data_indication.is_indication());
}


TEST_F(turn, create_permission)
{
  EXPECT_EQ(0x0008, turner::turn::create_permission.type());
  EXPECT_TRUE(turner::turn::create_permission.is_request());

  EXPECT_EQ(0x0108, turner::turn::create_permission_success.type());
  EXPECT_TRUE(turner::turn::create_permission_success.is_success_response());

  EXPECT_EQ(0x0118, turner::turn::create_permission_error.type());
  EXPECT_TRUE(turner::turn::create_permission_error.is_error_response());
}


TEST_F(turn, channel_bind)
{
  EXPECT_EQ(0x0009, turner::turn::channel_bind.type());
  EXPECT_TRUE(turner::turn::channel_bind.type());

  EXPECT_EQ(0x0109, turner::turn::channel_bind_success.type());
  EXPECT_TRUE(turner::turn::channel_bind_success.is_success_response());

  EXPECT_EQ(0x0119, turner::turn::channel_bind_error.type());
  EXPECT_TRUE(turner::turn::channel_bind_error.is_error_response());
}


}} // namespace turner_test
