#include <turner/common.test.hpp>
#include <turner/stun/message.hpp>


namespace turner_test { namespace {


namespace stun = turner::stun;


class turner_stun_message
  : public fixture
{
};


// TODO: STUN-specific tests only, common tests are covered by
// turner/message.test.cpp


TEST_F(turner_stun_message, ctor)
{
  stun::message_t message;
  EXPECT_FALSE(message.is_valid());
}


}} // namespace turner_test
