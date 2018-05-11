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
    turner::errc::insufficient_header_data,
    turner::errc::insufficient_payload_data,
    turner::errc::invalid_message_type,
    turner::errc::invalid_message_length,
    turner::errc::invalid_message_cookie,
    turner::errc::unexpected_message_type,
    turner::errc::unknown_comprehension_required,
    turner::errc::attribute_not_found,
    turner::errc::unexpected_attribute_length,
    turner::errc::unexpected_attribute_value,
    turner::errc::not_enough_room
  ),
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


TEST(error, success)
{
  EXPECT_EQ(0U, turner::success.code);
  EXPECT_EQ("Success", turner::success.message);

  constexpr turner::error_t ok{0, "Ok"};
  EXPECT_EQ(ok, turner::success);

  constexpr turner::error_t fail{500, "Fail"};
  EXPECT_NE(fail, turner::success);

  std::ostringstream oss;
  oss << turner::success;
  EXPECT_EQ("0 Success", oss.str());
}


}} // namespace turner_test
