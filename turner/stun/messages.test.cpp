#include <turner/common.test.hpp>


namespace turner_test { namespace {


using stun = turner_test::fixture;


TEST_F(stun, binding_name)
{
  EXPECT_STREQ("binding", turner::stun::binding.name());
  EXPECT_STREQ("binding_success", turner::stun::binding_success.name());
}


}} // namespace turner_test
