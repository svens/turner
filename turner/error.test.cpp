#include <turner/common.test.hpp>
#include <turner/error.hpp>


namespace turner_test { namespace {


TEST(error, category_name)
{
  EXPECT_STREQ("turner", turner::category().name());
}


TEST(error, make_error_code_invalid)
{
  auto error = turner::make_error_code(static_cast<turner::errc>(-1));
  ASSERT_TRUE(bool(error));

  EXPECT_EQ(-1, error.value());
  EXPECT_EQ(turner::category(), error.category());

  EXPECT_STREQ("turner", error.category().name());
  EXPECT_EQ("unknown error", error.message());
}


using errc = turner_test::with_value<turner::errc>;

INSTANTIATE_TEST_CASE_P(error, errc,
  testing::Values(
    turner::errc::insufficient_data,
    turner::errc::invalid_message_type,
    turner::errc::invalid_message_length,
    turner::errc::invalid_message_cookie
  )
);


TEST_P(errc, make_error_code)
{
  auto error = turner::make_error_code(GetParam());
  ASSERT_TRUE(bool(error));

  EXPECT_EQ(static_cast<int>(GetParam()), error.value());
  EXPECT_EQ(GetParam(), error);

  EXPECT_EQ(turner::category(), error.category());
  EXPECT_STREQ("turner", error.category().name());

  EXPECT_FALSE(error.message().empty());
  EXPECT_NE("unknown error", error.message());
}


}} // namespace turner_test
