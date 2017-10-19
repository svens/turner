#include <turner/common.test.hpp>
#include <turner/message.hpp>
#include <turner/message_type.hpp>
#include <turner/protocol.hpp>


namespace turner_test { namespace {


struct protocol
  : public turner_test::fixture
  , public turner::basic_protocol_t<protocol_t>
{
  virtual ~protocol () = default;
};


TEST_F(protocol, message_type)
{
  constexpr const auto msg_1 = message_type<1>();

  EXPECT_EQ(
    typeid(msg_1),
    typeid(turner::message_type_t<protocol_t, 1>)
  );

  EXPECT_EQ(uint16_t(1), msg_1);
  EXPECT_EQ(msg_1, uint16_t(1));

  EXPECT_EQ(uint16_t(0), msg_1.type() & turner::__bits::class_mask);
}


TEST_F(protocol, name)
{
  EXPECT_STREQ("Protocol", name());
}


TEST_F(protocol, name_unnamed)
{
  turner::basic_protocol_t<unnamed_protocol_t> p;
  EXPECT_EQ(nullptr, p.name());
}


TEST_F(protocol, ostream)
{
  std::ostringstream oss;
  oss << *this;
  EXPECT_EQ("Protocol", oss.str());
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
  auto msg = from_wire(raw, raw_end, error);
  EXPECT_TRUE(!error);
  EXPECT_TRUE(msg);
}


TEST_F(protocol, from_wire_first_nullptr)
{
  std::error_code error;
  auto msg = from_wire((const char *)0, raw, error);
  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_FALSE(msg);

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
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    from_wire(raw, (const char *)0),
    std::system_error
  );
}


TEST_F(protocol, from_wire_first_gt_last)
{
  std::error_code error;
  auto msg = from_wire(raw_end, raw, error);
  EXPECT_EQ(std::errc::invalid_argument, error);
  EXPECT_FALSE(msg);

  EXPECT_THROW(
    from_wire(raw_end, raw),
    std::system_error
  );
}


}} // namespace turner_test
