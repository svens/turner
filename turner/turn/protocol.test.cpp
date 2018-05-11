#include <turner/common.test.hpp>


namespace turner_test { namespace {


using turn = turner_test::fixture;


TEST_F(turn, parse_range)
{
  auto data = TURN::message_data;

  std::error_code error;
  turner::turn::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);

  EXPECT_NO_THROW(
    turner::turn::parse(data.begin(), data.end())
  );
}


TEST_F(turn, parse_range_with_error)
{
  auto data = TURN::message_data;
  data[0] = 0xff;

  std::error_code error;
  turner::turn::parse(data.begin(), data.end(), error);
  EXPECT_FALSE(!error);

  EXPECT_THROW(
    turner::turn::parse(data.begin(), data.end()),
    std::system_error
  );
}


TEST_F(turn, parse_data)
{
  auto data = TURN::message_data;

  std::error_code error;
  turner::turn::parse(data, error);
  EXPECT_TRUE(!error);

  EXPECT_NO_THROW(
    turner::turn::parse(data)
  );
}


TEST_F(turn, parse_data_with_error)
{
  auto data = TURN::message_data;
  data[0] = 0xff;

  std::error_code error;
  turner::turn::parse(data, error);
  EXPECT_FALSE(!error);

  EXPECT_THROW(
    turner::turn::parse(data),
    std::system_error
  );
}


}} // namespace turner_test
