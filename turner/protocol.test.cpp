#include <turner/common.test.hpp>
#include <turner/message.hpp>
#include <turner/protocol.hpp>
#include <array>
#include <string_view>


namespace turner_test { namespace {


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
};


struct protocol_t
  : public unnamed_protocol_t
{
  static constexpr std::string_view name () noexcept
  {
    using namespace std::string_view_literals;
    return "Protocol"sv;
  }
};


struct protocol
  : public turner_test::fixture
  , public turner::basic_protocol_t<protocol_t>
{};


const char raw[] =
  "\x00\x01"    // Type
  "\x00\x08"    // Length
  "_"
  "Cookie"
  "_"
  "Transaction"
  "_"
  "Payload"
  "\00";        // Padding


TEST_F(protocol, ostream)
{
  std::ostringstream oss;
  oss << *this;
  EXPECT_EQ(protocol_t::name(), oss.str());
}


TEST_F(protocol, ostream_unnamed)
{
  turner::basic_protocol_t<unnamed_protocol_t> p;
  std::ostringstream oss;
  oss << p;
  EXPECT_EQ("", oss.str());
}


TEST_F(protocol, from_wire)
{
  std::error_code error;
  auto msg = from_wire(raw, raw + sizeof(raw), error);
  ASSERT_TRUE(!error);
  EXPECT_FALSE(!msg);
}


TEST_F(protocol, from_wire_first_nullptr)
{
  std::error_code error;
  auto msg = from_wire((const char *)0, raw, error);
  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire((const char *)0, raw),
    std::system_error
  );
}


TEST_F(protocol, from_wire_last_nullptr)
{
  std::error_code error;
  auto msg = from_wire(raw, (const char *)0, error);
  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire(raw, (const char *)0),
    std::system_error
  );
}


TEST_F(protocol, from_wire_first_gt_last)
{
  std::error_code error;
  auto msg = from_wire(raw + sizeof(raw), raw, error);
  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire(raw + sizeof(raw), raw),
    std::system_error
  );
}


TEST_F(protocol, from_wire_insufficient_data)
{
  std::error_code error;
  auto msg = from_wire(raw, raw + protocol_t::header_size() / 2, error);
  EXPECT_EQ(turner::errc::insufficient_data, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire(raw, raw + protocol_t::header_size() / 2),
    std::system_error
  );
}


TEST_F(protocol, from_wire_type)
{
  std::error_code error;
  auto msg = from_wire(raw, raw + sizeof(raw), error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(1, msg.type());
}


TEST_F(protocol, from_wire_invalid_type)
{
  char data[sizeof(raw)];
  std::uninitialized_copy(raw, raw + sizeof(raw), data);
  data[0] |= 0b1100'0000;

  std::error_code error;
  auto msg = from_wire(data, data + sizeof(data), error);
  EXPECT_EQ(turner::errc::invalid_message_type, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire(data, data + sizeof(data)),
    std::system_error
  );
}


TEST_F(protocol, from_wire_length)
{
  std::error_code error;
  auto msg = from_wire(raw, raw + sizeof(raw), error);
  ASSERT_TRUE(!error);
  EXPECT_EQ(8, msg.length());
}


TEST_F(protocol, from_wire_invalid_length)
{
  char data[sizeof(raw)];
  std::uninitialized_copy(raw, raw + sizeof(raw), data);
  data[3] += 1;

  std::error_code error;
  auto msg = from_wire(data, data + sizeof(data), error);
  EXPECT_EQ(turner::errc::invalid_message_length, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire(data, data + sizeof(data)),
    std::system_error
  );
}


TEST_F(protocol, from_wire_invalid_cookie)
{
  char data[sizeof(raw)];
  std::uninitialized_copy(raw, raw + sizeof(raw), data);
  data[protocol_t::cookie_offset()] += 1;

  std::error_code error;
  auto msg = from_wire(data, data + sizeof(data), error);
  EXPECT_EQ(turner::errc::invalid_message_cookie, error);
  EXPECT_TRUE(!msg);

  EXPECT_THROW(
    from_wire(data, data + sizeof(data)),
    std::system_error
  );
}


}} // namespace turner_test
