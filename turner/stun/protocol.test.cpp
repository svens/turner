#include <turner/common.test.hpp>


namespace turner_test { namespace {


using stun = turner_test::fixture;


TEST_F(stun, parse_range)
{
  auto data = STUN::msg_data();

  std::error_code error;
  turner::stun::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);

  EXPECT_NO_THROW(
    turner::stun::parse(data.begin(), data.end())
  );
}


TEST_F(stun, parse_range_with_error)
{
  auto data = STUN::msg_data();
  data[0] = 0xff;

  std::error_code error;
  turner::stun::parse(data.begin(), data.end(), error);
  EXPECT_FALSE(!error);

  EXPECT_THROW(
    turner::stun::parse(data.begin(), data.end()),
    std::system_error
  );
}


TEST_F(stun, parse_data)
{
  auto data = STUN::msg_data();

  std::error_code error;
  turner::stun::parse(data, error);
  EXPECT_TRUE(!error);

  EXPECT_NO_THROW(
    turner::stun::parse(data)
  );
}


TEST_F(stun, parse_data_with_error)
{
  auto data = STUN::msg_data();
  data[0] = 0xff;

  std::error_code error;
  turner::stun::parse(data, error);
  EXPECT_FALSE(!error);

  EXPECT_THROW(
    turner::stun::parse(data),
    std::system_error
  );
}


}} // namespace turner_test
