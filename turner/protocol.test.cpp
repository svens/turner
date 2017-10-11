#include <turner/common.test.hpp>
#include <turner/protocol.hpp>


namespace turner_test { namespace {


using protocol = turner_test::fixture;


struct traits_t
{
  // cookie
  static __turner_inline_var constexpr std::array<uint8_t, 6> cookie =
  {{
    'c', 'o', 'o', 'k', 'i', 'e',
  }};
  static __turner_inline_var constexpr size_t cookie_size = cookie.size();
  static __turner_inline_var constexpr size_t cookie_offset = 5;

  // transaction id
  static __turner_inline_var constexpr size_t transaction_id_size = 11;
  static __turner_inline_var constexpr size_t transaction_id_offset = 12;
};

using my_protocol_t = turner::basic_protocol_t<traits_t>;

__turner_inline_var constexpr const auto my_protocol = my_protocol_t{};


__turner_inline_var std::string_view my_protocol_message =
  "\00\01\00\07_cookie_transaction_Payload";


TEST_F(protocol, traits)
{
  ::testing::StaticAssertTypeEq<
    my_protocol_t::traits_t,
    traits_t
  >();
}


TEST_F(protocol, type)
{
  uint16_t expected = 1;
  EXPECT_EQ(expected, my_protocol.type(my_protocol_message.begin()));
  EXPECT_NE(expected, my_protocol.type(my_protocol_message.begin() + 1));
}


TEST_F(protocol, length)
{
  uint16_t expected = 7;
  EXPECT_EQ(expected, my_protocol.length(my_protocol_message.begin()));
  EXPECT_NE(expected, my_protocol.length(my_protocol_message.begin() + 1));
}


TEST_F(protocol, cookie)
{
  auto expected = traits_t::cookie;
  EXPECT_EQ(expected, my_protocol.cookie(my_protocol_message.begin()));
  EXPECT_NE(expected, my_protocol.cookie(my_protocol_message.begin() + 1));
}


TEST_F(protocol, transaction_id)
{
  static constexpr const std::array<uint8_t, 11> expected =
  {{
     't', 'r', 'a', 'n', 's', 'a', 'c', 't', 'i', 'o', 'n',
  }};
  EXPECT_EQ(expected, my_protocol.transaction_id(my_protocol_message.begin()));
  EXPECT_NE(expected, my_protocol.transaction_id(my_protocol_message.begin() + 1));
}


}} // namespace turner_test
