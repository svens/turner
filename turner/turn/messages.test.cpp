#include <turner/common.test.hpp>


namespace turner_test { namespace {


using turn = turner_test::fixture;


TEST_F(turn, allocation_name)
{
  EXPECT_STREQ("allocation", turner::turn::allocation.name());
  EXPECT_STREQ("allocation_success", turner::turn::allocation_success.name());
  EXPECT_STREQ("allocation_error", turner::turn::allocation_error.name());
}


TEST_F(turn, refresh_name)
{
  EXPECT_STREQ("refresh", turner::turn::refresh.name());
  EXPECT_STREQ("refresh_success", turner::turn::refresh_success.name());
  EXPECT_STREQ("refresh_error", turner::turn::refresh_error.name());
}


TEST_F(turn, send_indication_name)
{
  EXPECT_STREQ("send_indication", turner::turn::send_indication.name());
}


TEST_F(turn, data_indication_name)
{
  EXPECT_STREQ("data_indication", turner::turn::data_indication.name());
}


TEST_F(turn, create_permission_name)
{
  EXPECT_STREQ("create_permission", turner::turn::create_permission.name());
  EXPECT_STREQ("create_permission_success", turner::turn::create_permission_success.name());
  EXPECT_STREQ("create_permission_error", turner::turn::create_permission_error.name());
}


TEST_F(turn, channel_bind_name)
{
  EXPECT_STREQ("channel_bind", turner::turn::channel_bind.name());
  EXPECT_STREQ("channel_bind_success", turner::turn::channel_bind_success.name());
  EXPECT_STREQ("channel_bind_error", turner::turn::channel_bind_error.name());
}


}} // namespace turner_test
