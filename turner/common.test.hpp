#pragma once

#include <turner/config.hpp>
#include <turner/stun/stun.hpp>
#include <turner/turn/turn.hpp>

#define GTEST_HAS_TR1_TUPLE 0
#include <gtest/gtest.h>


//
// protocol types are intentionally out of namespace for gtest TypeParam tests
//

struct STUN: public turner::stun::protocol_t
{
  static constexpr const char expected_name[] = "STUN";
};

struct TURN: public turner::turn::protocol_t
{
  static constexpr const char expected_name[] = "TURN";
};


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


using protocol_types = ::testing::Types<
  STUN,
  TURN
>;


template <typename Protocol>
class with_protocol
  : public with_type<Protocol>
{};


// STUN {{{1

inline auto msg_data (STUN)
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
    0x00, 0x00,
  }};
}

inline constexpr auto msg_type (STUN)
{
  return turner::stun::binding;
}

inline constexpr auto msg_type_v (STUN)
{
  return turner::stun::binding.type();
}

inline constexpr auto msg_len (STUN)
{
  return 8;
}

inline constexpr auto msg_txn_id (STUN)
{
  return turner::stun::protocol_t::transaction_id_t
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
  }};
}

inline auto msg_hmac (STUN)
{
  return turner::stun::make_integrity_calculator(
    "realm",
    "user",
    "pass"
  );
}

inline constexpr auto msg_success_type (STUN)
{
  return turner::stun::binding_success;
}

inline constexpr auto msg_error_type (STUN)
{
  // there is no error response for STUN Binding
  // let's invent it for testing
  return turner::stun::binding_t::error_response_t{};
}


// TURN {{{1

inline auto msg_data (TURN)
{
  return std::vector<uint8_t>
  {{
    // header
    0x00, 0x03,                 // Type (Allocation)
    0x00, 0x08,                 // Length
    0x21, 0x12, 0xa4, 0x42,     // Cookie
    0x00, 0x01, 0x02, 0x03,     // Transaction ID
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,

    // attributes
    0x00, 0x01,                 // Type (XXX)
    0x00, 0x01,                 // Length
    0x01, 0x00,                 // Value + padding
    0x00, 0x00,
  }};
}

inline constexpr auto msg_type (TURN)
{
  return turner::turn::allocation;
}

inline constexpr auto msg_type_v (TURN)
{
  return turner::turn::allocation.type();
}

inline constexpr auto msg_len (TURN)
{
  return 8;
}

inline constexpr auto msg_txn_id (TURN)
{
  return turner::turn::protocol_t::transaction_id_t
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
  }};
}

inline auto msg_hmac (TURN)
{
  return turner::turn::make_integrity_calculator(
    "realm",
    "user",
    "pass"
  );
}

inline constexpr auto msg_success_type (TURN)
{
  return turner::turn::allocation_success;
}

inline constexpr auto msg_error_type (TURN)
{
  return turner::turn::allocation_error;
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
using unnamed_protocol_t = turner::protocol_t<unnamed_protocol_traits_t>;
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


template <typename Protocol, size_t N>
auto wire_data (Protocol protocol, const char (&data)[N])
{
  // get message and remove existing body
  auto raw = msg_data(protocol);
  raw.resize(Protocol::traits_t::header_size);

  // append new body and update message length
  raw.insert(raw.end(), data, data + N - 1);
  reinterpret_cast<uint16_t *>(&raw[0])[1] =
    sal::native_to_network_byte_order((uint16_t)(N - 1));

  return raw;
}


template <typename Protocol, typename Data>
inline auto &parse (Protocol protocol, const Data &d)
{
  return Protocol::parse(d.begin(), d.end())->as(msg_type(protocol));
}


template <typename Protocol, typename Data>
inline auto build (Protocol protocol, Data &d)
{
  return Protocol::build(msg_type(protocol), d.begin(), d.end());
}


} // namespace turner_test
