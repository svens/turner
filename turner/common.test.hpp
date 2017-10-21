#pragma once

#include <turner/config.hpp>
#include <turner/stun.hpp>
#include <turner/message.hpp>

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


// Test data {{{1

// note: functions returning protocol specific test data are prefixed with x_
// to differentiate them from test case names

using STUN = turner::stun::protocol_t;

using protocol_types = ::testing::Types<
  STUN
>;

template <typename Protocol>
class with_protocol
  : public with_type<Protocol>
{};


// STUN {{{1

inline auto x_request (STUN)
{
  return std::vector<uint8_t>
  {{
    // header
    0x00, 0x01,                 // Type (Binding)
    0x00, 0x08,                 // Length
    0x21, 0x12, 0xa4, 0x42,     // Cookie
    0x00, 0x01, 0x02, 0x03,     // Transaction ID
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,

    // attributes
    0x00, 0x01,                 // Type (XXX)
    0x00, 0x01,                 // Length
    0x01, 0x00,                 // Value + padding
    0x00, 0x00
  }};
}

inline constexpr auto x_message_type (STUN)
{
  return turner::stun::binding;
}

inline constexpr auto x_message_type_value (STUN)
{
  return turner::stun::binding.type();
}

inline constexpr auto x_message_length (STUN)
{
  return 8;
}

inline constexpr auto x_transaction_id (STUN)
{
  return STUN::transaction_id_t
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
  }};
}


// Unnamed protocol for testing {{{1

// unused message type (0 is unused for STUN/TURN/MSTURN)
template <typename Protocol>
using unused_message_type_t = typename Protocol::template message_type_t<0>;

template <typename Protocol>
static __turner_inline_var constexpr const unused_message_type_t<Protocol>
  unused_message_type{};

// protocol
struct unnamed_protocol_traits_t : public turner::stun::protocol_traits_t {};
using unnamed_protocol_t = turner::basic_protocol_t<unnamed_protocol_traits_t>;
static __turner_inline_var constexpr const unnamed_protocol_t unnamed_protocol;

// message type
using unnamed_protocol_message_type_t = unnamed_protocol_t::message_type_t<1>;
static __turner_inline_var constexpr const unnamed_protocol_message_type_t
  unnamed_protocol_message_type;

inline constexpr void operator>> (unnamed_protocol_message_type_t,
  const char *&name) noexcept
{
  name = "unnamed_protocol_message";
}


//}}}1


} // namespace turner_test
