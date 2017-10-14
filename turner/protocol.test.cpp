#include <turner/common.test.hpp>
#include <turner/protocol.hpp>
#include <turner/message_type.hpp>


namespace turner_test { namespace {


extern const char protocol_name[] = "TestProtocol";

struct protocol
  : public turner_test::fixture
  , public turner::basic_protocol_t<protocol, protocol_name>
{};

struct unnamed_protocol_t
  : public turner::basic_protocol_t<unnamed_protocol_t>
{};


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
  EXPECT_EQ(protocol_name, oss.str());
}


TEST_F(protocol, ostream_unnamed)
{
  std::ostringstream oss;
  oss << 'a' << unnamed_protocol_t{} << 'b';
  EXPECT_EQ("ab", oss.str());
}


TEST_F(protocol, name)
{
  EXPECT_STREQ(protocol_name, name());
  EXPECT_EQ(nullptr, unnamed_protocol_t::name());
}


extern const char msg_type_name[] = "message_type";

TEST_F(protocol, message_type)
{
  auto msg_type = message_type<0x001, msg_type_name>();

  ::testing::StaticAssertTypeEq<
    decltype(msg_type)::protocol_t,
    protocol
  >();

  EXPECT_EQ(uint16_t(0x001), msg_type);
  EXPECT_STREQ(msg_type_name, msg_type.name());

  std::ostringstream oss;
  oss << msg_type;
  EXPECT_EQ(msg_type_name, oss.str());
}


}} // namespace turner_test
