#include <turner/common.test.hpp>
#include <turner/stun/message.hpp>
#include <turner/error.hpp>


namespace turner_test { namespace {


template <typename Protocol>
using turner_message = turner_test::with_type<Protocol>;

TYPED_TEST_CASE(turner_message, protocol_types);


TYPED_TEST(turner_message, ctor)
{
  turner::basic_message_t<TypeParam> message;
  EXPECT_FALSE(message.is_valid());
  EXPECT_TRUE(!message);
}


TYPED_TEST(turner_message, ctor_valid_message)
{
  std::error_code error;
  auto message = TypeParam::message(
    valid_message<TypeParam>,
    valid_message<TypeParam> + sizeof(valid_message<TypeParam>),
    error
  );
  EXPECT_TRUE(!error);
  EXPECT_TRUE(message.is_valid());
}


TYPED_TEST(turner_message, ctor_invalid_message)
{
  std::error_code error;
  auto message = TypeParam::message(
    invalid_message<TypeParam>,
    invalid_message<TypeParam> + sizeof(invalid_message<TypeParam>),
    error
  );
  EXPECT_EQ(turner::errc::illegal_byte_sequence, error);
  EXPECT_FALSE(message.is_valid());
}


TYPED_TEST(turner_message, transaction_id)
{
  std::error_code error;
  auto message = TypeParam::message(
    valid_message<TypeParam>,
    valid_message<TypeParam> + sizeof(valid_message<TypeParam>),
    error
  );
  ASSERT_TRUE(message.is_valid());
  EXPECT_EQ(expected_transaction_id<TypeParam>, message.transaction_id());
}


}} // namespace turner_test
