#pragma once

#include <turner/config.hpp>
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
  static constexpr uint16_t header_size () noexcept
  {
    return 24;
  }

  static constexpr size_t cookie_offset () noexcept
  {
    return 5;
  }

  static constexpr std::array<uint8_t, 6> cookie () noexcept
  {
    return
    {
      { 'C', 'o', 'o', 'k', 'i', 'e', }
    };
  }

  static constexpr size_t transaction_id_offset () noexcept
  {
    return 12;
  }

  static constexpr size_t transaction_id_size () noexcept
  {
    return 11;
  }
};


struct protocol_t
  : public unnamed_protocol_t
{};


inline constexpr void operator>> (turner::basic_protocol_t<protocol_t>,
  const char *&name) noexcept
{
  name = "Protocol";
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
