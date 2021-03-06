#pragma once

#include <turner/config.hpp>
#include <turner/msturn/msturn.hpp>
#include <turner/stun/stun.hpp>
#include <turner/turn/turn.hpp>
#include <gtest/gtest.h>


//
// protocol types are intentionally out of namespace for gtest TypeParam tests
//

struct MSTURN: public turner::msturn::protocol_t //{{{1
{
  static auto message_hmac ()
  {
    return turner::msturn::make_integrity_calculator(
      "realm",
      "user",
      "pass"
    );
  }

  static auto message_invalid_hmac ()
  {
    return turner::msturn::make_integrity_calculator(
      "realm",
      "user",
      "invalid"
    );
  }

  static constexpr auto message = turner::msturn::allocate;
  static constexpr auto message_v = message.type;
  static constexpr auto message_length = 0x20;

  static constexpr auto success_message = turner::msturn::allocate_success;
  static constexpr auto error_message = turner::msturn::allocate_error;

  static constexpr auto username = turner::msturn::username;
  static constexpr auto software = turner::msturn::software;

  static constexpr bool expect_framing_header = true;

  static constexpr turner::msturn::protocol_t::transaction_id_t message_txn =
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,
  }};

  static inline const std::vector<uint8_t> message_data =
  {
    // header
    0x00, 0x03, 0x00, 0x20,     // Type (Allocation), Length
    0x00, 0x01, 0x02, 0x03,     // Transaction ID
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f,

    // Magic Cookie
    0x00, 0x0f, 0x00, 0x04,
    0x72, 0xc6, 0x4b, 0xc6,

    // Message Integrity
    0x00, 0x08, 0x00, 0x14,     // Type, Length
    0xf4, 0xaf, 0x03, 0xc6,     // 20B HMAC-SHA1
    0x21, 0x6f, 0x19, 0x76,
    0x65, 0x9f, 0x87, 0x21,
    0x18, 0x7c, 0xff, 0xc5,
    0x9e, 0xb7, 0x32, 0x2a,
  };
};


struct STUN: public turner::stun::protocol_t //{{{1
{
  static auto message_hmac ()
  {
    return turner::stun::make_integrity_calculator(
      "realm",
      "user",
      "pass"
    );
  }

  static auto message_invalid_hmac ()
  {
    return turner::stun::make_integrity_calculator(
      "realm",
      "user",
      "invalid"
    );
  }

  static constexpr auto message = turner::stun::binding;
  static constexpr auto message_v = message.type;
  static constexpr auto message_length = 0x18;

  static constexpr auto success_message = turner::stun::binding_success;
  static constexpr auto error_message = turner::stun::protocol_t::message_type_t<0x0111>{};

  static constexpr auto username = turner::stun::username;
  static constexpr auto software = turner::stun::software;

  static constexpr bool expect_framing_header = false;

  static constexpr turner::stun::protocol_t::transaction_id_t message_txn =
  {{
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
  }};

  static inline const std::vector<uint8_t> message_data =
  {
    // header
    0x00, 0x01, 0x00, 0x18,     // Type (Binding), Length
    0x21, 0x12, 0xa4, 0x42,     // Cookie
    0x00, 0x01, 0x02, 0x03,     // Transaction ID
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,

    // MESSAGE-INTEGRITY
    0x00, 0x08, 0x00, 0x14,     // Type, Length
    0xfb, 0xb7, 0x7f, 0xe5,     // 20B HMAC-SHA1
    0x34, 0xa2, 0x4b, 0xfa,
    0xa5, 0xf1, 0x52, 0x65,
    0x93, 0xfe, 0xf6, 0x59,
    0x9e, 0x9b, 0x64, 0x28,
  };
};


struct TURN: public turner::turn::protocol_t //{{{1
{
  static auto message_hmac ()
  {
    return turner::turn::make_integrity_calculator(
      "realm",
      "user",
      "pass"
    );
  }

  static auto message_invalid_hmac ()
  {
    return turner::turn::make_integrity_calculator(
      "realm",
      "user",
      "invalid"
    );
  }

  static constexpr auto message = turner::turn::allocation;
  static constexpr auto message_v = message.type;
  static constexpr auto message_length = 0x18;

  static constexpr auto success_message = turner::turn::allocation_success;
  static constexpr auto error_message = turner::turn::allocation_error;

  static constexpr auto username = turner::turn::username;
  static constexpr auto software = turner::turn::software;

  static constexpr bool expect_framing_header = false;

  static constexpr turner::turn::protocol_t::transaction_id_t message_txn =
  {{
     0x00, 0x01, 0x02, 0x03,
     0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0a, 0x0b,
  }};

  static inline const std::vector<uint8_t> message_data =
  {
    // header
    0x00, 0x03, 0x00, 0x18,     // Type (Allocation), Length
    0x21, 0x12, 0xa4, 0x42,     // Cookie
    0x00, 0x01, 0x02, 0x03,     // Transaction ID
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,

    // MESSAGE-INTEGRITY
    0x00, 0x08, 0x00, 0x14,     // Type, Length
    0x3f, 0x74, 0xd2, 0x2f,     // 20B HMAC-SHA1
    0x2a, 0xed, 0xfe, 0xc3,
    0xe0, 0x82, 0x27, 0x29,
    0x6e, 0x3c, 0x22, 0x49,
    0x4a, 0x15, 0x2c, 0x23,
  };
};


//}}}1


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
  MSTURN,
  STUN,
  TURN
>;


template <typename Protocol>
class with_protocol
  : public with_type<Protocol>
{};


template <typename Protocol, size_t N>
auto wire_data (Protocol, const char (&data)[N])
{
  // get message and remove existing payload
  auto raw = Protocol::message_data;
  raw.resize(Protocol::header_and_cookie_size());

  // update length
  reinterpret_cast<uint16_t *>(&raw[0])[1] =
    sal::native_to_network_byte_order(
      static_cast<uint16_t>(N - 1 + Protocol::min_payload_length())
    );

  // append new payload
  raw.insert(raw.end(), data, data + N - 1);
  return raw;
}


template <typename Protocol, typename Data>
inline auto &parse (Protocol, const Data &d)
{
  return Protocol::parse(d, sal::throw_on_error("test::parse"))
    ->as(Protocol::message)
  ;
}


template <typename Protocol, typename Data>
inline auto build (Protocol, Data &d)
{
  return Protocol::message
    .make(d, sal::throw_on_error("test::build"))
  ;
}


} // namespace turner_test
