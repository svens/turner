#include <turner/common.test.hpp>
#include <turner/protocol.hpp>
#include <string_view>


namespace turner_test { namespace {


struct protocol
  : public turner_test::fixture
  , public turner::basic_protocol_t<protocol>
{
  static constexpr std::string_view name () noexcept
  {
    using namespace std::string_view_literals;
    return "Protocol"sv;
  }
};


const char message[] =
  "\00\01\00\07"
  "_"
  "cookie"
  "_"
  "transaction"
  "_"
  "Payload";


TEST_F(protocol, type)
{
  uint16_t expected = 1;
  EXPECT_EQ(expected, type(message));
  EXPECT_NE(expected, type(message + 1));
}


TEST_F(protocol, length)
{
  uint16_t expected = 7;
  EXPECT_EQ(expected, length(message));
  EXPECT_NE(expected, length(message + 1));
}


TEST_F(protocol, ostream)
{
  std::ostringstream oss;
  oss << *this;
  EXPECT_EQ(name(), oss.str());
}


TEST_F(protocol, ostream_unnamed)
{
  struct unnamed_protocol_t
    : public turner::basic_protocol_t<unnamed_protocol_t>
  {};

  std::ostringstream oss;
  oss << unnamed_protocol_t{};
  EXPECT_EQ("", oss.str());
}


}} // namespace turner_test
