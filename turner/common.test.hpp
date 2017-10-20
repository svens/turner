#pragma once

#include <turner/config.hpp>
#include <turner/message_type.hpp>
#include <turner/protocol.hpp>
#include <array>

#define GTEST_HAS_TR1_TUPLE 0
#include <gtest/gtest.h>


namespace turner_test {


class fixture
  : public ::testing::Test
{
public:

  const std::string case_name = case_name_();


private:

  static std::string case_name_ ()
  {
    if (auto *info = ::testing::UnitTest::GetInstance()->current_test_info())
    {
      std::string name = info->test_case_name();
      name += '.';
      name += info->name();
      return name;
    }
    return "<unknown>";
  }
};


template <typename T>
class with_type
  : public fixture
{};


template <typename T>
class with_value
  : public fixture
  , public ::testing::WithParamInterface<T>
{};


// Protocol and related data for testing //{{{1


struct unnamed_protocol_t
{
  static __turner_inline_var constexpr uint16_t header_size = 24;

  static __turner_inline_var constexpr size_t cookie_offset = 5;

  static __turner_inline_var constexpr std::array<uint8_t, 6> cookie =
  {
    { 'C', 'o', 'o', 'k', 'i', 'e', }
  };

  static __turner_inline_var constexpr size_t transaction_id_offset = 12;

  static __turner_inline_var constexpr size_t transaction_id_size = 11;
};


using up_msg_1_t = turner::message_type_t<turner_test::unnamed_protocol_t, 1>;
__turner_inline_var constexpr const up_msg_1_t up_msg_1{};


struct protocol_t
  : public unnamed_protocol_t
{
  template <uint16_t Type>
  using message_type_t = turner::message_type_t<protocol_t, Type>;
};


inline constexpr void operator>> (turner::basic_protocol_t<protocol_t>,
  const char *&name) noexcept
{
  name = "Protocol";
}


using msg_1_t = protocol_t::message_type_t<1>;
__turner_inline_var constexpr const msg_1_t msg_1{};

using msg_1a_t = protocol_t::message_type_t<1>;
__turner_inline_var constexpr const msg_1a_t msg_1a{};

using msg_2_t = protocol_t::message_type_t<2>;
__turner_inline_var constexpr const msg_2_t msg_2{};


inline constexpr void operator>> (decltype(msg_1), const char *&name) noexcept
{
  name = "msg_1";
}


__turner_inline_var constexpr const char raw[] =
  "\x00\x01"    // Type
  "\x00\x08"    // Length
  "_"
  "Cookie"
  "_"
  "Transaction"
  "_"
  "Payload"
  "\00";        // Padding

__turner_inline_var constexpr const char *raw_end = raw + sizeof(raw) - 1;


//}}}1


} // namespace turner_test
