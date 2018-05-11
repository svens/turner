#include <turner/common.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using protocol = turner_test::with_protocol<Protocol>;

TYPED_TEST_CASE(protocol, protocol_types);


TYPED_TEST(protocol, framing_header)
{
  EXPECT_EQ(
    TypeParam::expect_framing_header,
    TypeParam::has_stream_framing_header
  );
}


TYPED_TEST(protocol, parse)
{
  auto data = TypeParam::message_data;

  std::error_code error;
  auto msg = TypeParam::parse(data.begin(), data.end(), error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(msg);

  EXPECT_EQ(TypeParam::message, msg->type());
}


}} // namespace turner_test
