#include <turner/common.test.hpp>


namespace turner_test { namespace {


using msturn = turner_test::fixture;


TEST_F(msturn, parse_range)
{
  auto data = MSTURN::msg_data();

  std::error_code error;
  turner::msturn::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);

  EXPECT_NO_THROW(
    turner::msturn::parse(data.begin(), data.end())
  );
}


TEST_F(msturn, parse_range_with_error)
{
  auto data = MSTURN::msg_data();
  data[0] = 0xff;

  std::error_code error;
  turner::msturn::parse(data.begin(), data.end(), error);
  EXPECT_FALSE(!error);

  EXPECT_THROW(
    turner::msturn::parse(data.begin(), data.end()),
    std::system_error
  );
}


TEST_F(msturn, parse_data)
{
  auto data = MSTURN::msg_data();

  std::error_code error;
  turner::msturn::parse(data, error);
  EXPECT_TRUE(!error);

  EXPECT_NO_THROW(
    turner::msturn::parse(data)
  );
}


TEST_F(msturn, parse_data_with_error)
{
  auto data = MSTURN::msg_data();
  data[0] = 0xff;

  std::error_code error;
  turner::msturn::parse(data, error);
  EXPECT_FALSE(!error);

  EXPECT_THROW(
    turner::msturn::parse(data),
    std::system_error
  );
}


}} // namespace turner_test
