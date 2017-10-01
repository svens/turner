#pragma once

#include <turner/msturn.hpp>
#include <turner/stun.hpp>
#include <turner/turn.hpp>

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


using MSTURN = turner::msturn::protocol_t;
using STUN = turner::stun::protocol_t;
using TURN = turner::turn::protocol_t;

using protocol_types = testing::Types<
  MSTURN,
  STUN,
  TURN
>;


//
// Test data
//


// MS-TURN {{{1


inline constexpr auto expected_request (MSTURN)
{
  return turner::msturn::allocate;
}


inline constexpr auto expected_success_response (MSTURN)
{
  return turner::msturn::allocate_success;
}


inline constexpr uint16_t expected_length (MSTURN)
{
  return 0U;
}


inline constexpr auto request (MSTURN)
{
  return std::array<uint8_t, 28>
  {{
    0x00, 0x03,                   // MS-TURN Allocate
    0x00, 0x00,                   // length
    0x00, 0x01, 0x02, 0x03,       // transaction id
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x0f,                   // cookie as TLV
    0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,
 }};
}


inline constexpr auto success_response (MSTURN)
{
  return std::array<uint8_t, 28>
  {{
    0x01, 0x03,                   // MS-TURN Allocate success response
    0x00, 0x00,                   // length
    0x00, 0x01, 0x02, 0x03,       // transaction id
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x0f,                   // cookie as TLV
    0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,
  }};
}


inline constexpr auto expected_transaction_id (MSTURN)
{
  return turner::msturn::message_t::transaction_id_t
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
  }};
}


inline constexpr auto message_with_bad_type (MSTURN)
{
  return std::array<uint8_t, 28>
  {{
    0x00, 0x02,                   // invalid MS-TURN method
    0x00, 0x00,                   // length
    0x00, 0x01, 0x02, 0x03,       // transaction id
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x0f,                   // cookie as TLV
    0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,
  }};
}


inline constexpr auto message_with_bad_length (MSTURN)
{
  return std::array<uint8_t, 28>
  {{
    0x00, 0x03,                   // MS-TURN Allocate
    0x00, 0x11,                   // length
    0x00, 0x01, 0x02, 0x03,       // transaction id
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x0f,                   // cookie as TLV
    0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,
  }};
}


inline constexpr auto message_with_valid_length_and_insufficient_data (MSTURN)
{
  return std::array<uint8_t, 30>
  {{
    0x00, 0x03,                   // MS-TURN Allocate
    0x00, 0x04,                   // length
    0x00, 0x01, 0x02, 0x03,       // transaction id
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x0f,                   // cookie as TLV
    0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,
    0x01, 0x02,                   // insufficient data
  }};
}


inline constexpr auto message_with_bad_cookie (MSTURN)
{
  return std::array<uint8_t, 28>
  {{
    0x00, 0x03,                   // MS-TURN Allocate
    0x00, 0x00,                   // length
    0x00, 0x01, 0x02, 0x03,       // transaction id
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
    0x01, 0x0f,                   // bad cookie as TLV
    0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,
  }};
}


// STUN {{{1


inline constexpr auto expected_request (STUN)
{
  return turner::stun::binding;
}


inline constexpr auto expected_success_response (STUN)
{
  return turner::stun::binding_success;
}


inline constexpr uint16_t expected_length (STUN)
{
  return 0U;
}


inline constexpr auto request (STUN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x01,                   // STUN Binding
    0x00, 0x00,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
 }};
}


inline constexpr auto success_response (STUN)
{
  return std::array<uint8_t, 20>
  {{
    0x01, 0x01,                   // STUN Binding succes response
    0x00, 0x00,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto expected_transaction_id (STUN)
{
  return turner::stun::message_t::transaction_id_t
  {{
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto message_with_bad_type (STUN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x02,                   // invalid STUN method
    0x00, 0x00,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto message_with_bad_length (STUN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x01,                   // STUN Binding
    0x00, 0x11,                   // bad length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto message_with_valid_length_and_insufficient_data (STUN)
{
  return std::array<uint8_t, 22>
  {{
    0x00, 0x01,                   // STUN Binding
    0x00, 0x04,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
    0x01, 0x02,                   // insufficient data
  }};
}


inline constexpr auto message_with_bad_cookie (STUN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x01,                   // STUN Binding
    0x00, 0x00,                   // length
    0x12, 0x12, 0xa4, 0x42,       // invalid cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


// TURN {{{1


inline constexpr auto expected_request (TURN)
{
  return turner::turn::allocation;
}


inline constexpr auto expected_success_response (TURN)
{
  return turner::turn::allocation_success;
}


inline constexpr uint16_t expected_length (TURN)
{
  return 0U;
}


inline constexpr auto request (TURN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x03,                   // TURN Allocation
    0x00, 0x00,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
 }};
}


inline constexpr auto success_response (TURN)
{
  return std::array<uint8_t, 20>
  {{
    0x01, 0x03,                   // TURN Allocation succes response
    0x00, 0x00,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto expected_transaction_id (TURN)
{
  return turner::stun::message_t::transaction_id_t
  {{
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto message_with_bad_type (TURN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x02,                   // invalid TURN method
    0x00, 0x00,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto message_with_bad_length (TURN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x03,                   // TURN Allocation
    0x00, 0x11,                   // bad length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


inline constexpr auto message_with_valid_length_and_insufficient_data (TURN)
{
  return std::array<uint8_t, 22>
  {{
    0x00, 0x03,                   // TURN Allocation
    0x00, 0x04,                   // length
    0x21, 0x12, 0xa4, 0x42,       // cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
    0x01, 0x02,                   // insufficient data
  }};
}


inline constexpr auto message_with_bad_cookie (TURN)
{
  return std::array<uint8_t, 20>
  {{
    0x00, 0x03,                   // TURN Allocation
    0x00, 0x00,                   // length
    0x12, 0x12, 0xa4, 0x42,       // invalid cookie
    0x01, 0x02, 0x03, 0x04,       // transaction id
    0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12,
  }};
}


// }}}1


} // namespace turner_test
