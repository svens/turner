#pragma once

#include <turner/stun.hpp>

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
{
};


template <typename T>
class with_value
  : public fixture
  , public ::testing::WithParamInterface<T>
{
};


using protocol_types = testing::Types<
  turner::stun::protocol_t
>;


//
// Test data
//


// Protocol independent {{{1


template <typename Protocol> bool valid_message = {};
template <typename Protocol> bool invalid_message = {};
template <typename Protocol> bool expected_transaction_id = {};


// STUN specializations {{{1


template <>
inline uint8_t valid_message<turner::stun::protocol_t>[] =
{
  1
};

template <>
inline uint8_t invalid_message<turner::stun::protocol_t>[] =
{
  2
};

template <>
inline turner::stun::message_t::transaction_id_t
  expected_transaction_id<turner::stun::protocol_t> =
{
};


// }}}1


} // namespace turner_test
