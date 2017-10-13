#include <turner/common.test.hpp>
#include <turner/message_type.hpp>
#include <turner/protocol.hpp>


namespace turner_test { namespace {


using message_type = turner_test::fixture;

extern const char foo_name[] = "foo";
struct foo_t
  : public turner::basic_protocol_t<foo_t, foo_name>
{};

extern const char a_name[] = "a";
using a_t = turner::basic_message_type_t<foo_t, 0x001, a_name>;
constexpr const a_t a{};

extern const char b_name[] = "b";
using b_t = turner::basic_message_type_t<foo_t, 0x001, b_name>;
constexpr const b_t b{};

using c_t = turner::basic_message_type_t<foo_t, 0x002>;
constexpr const c_t c{};

using d_t = turner::basic_message_type_t<foo_t, 0x002>;
constexpr const d_t d{};

struct bar_t
  : public turner::basic_protocol_t<bar_t>
{};

using e_t = turner::basic_message_type_t<bar_t, 0x001>;
constexpr const e_t e{};


TEST_F(message_type, protocol)
{
  ::testing::StaticAssertTypeEq<a_t::protocol_t, foo_t>();
  ::testing::StaticAssertTypeEq<b_t::protocol_t, foo_t>();
  ::testing::StaticAssertTypeEq<c_t::protocol_t, foo_t>();
  ::testing::StaticAssertTypeEq<d_t::protocol_t, foo_t>();

  ::testing::StaticAssertTypeEq<e_t::protocol_t, bar_t>();
}


TEST_F(message_type, types)
{
  // expected compilation failures
  // ::testing::StaticAssertTypeEq<a_t, b_t>();
  // ::testing::StaticAssertTypeEq<a_t, c_t>();
  // ::testing::StaticAssertTypeEq<a_t, d_t>();

  ::testing::StaticAssertTypeEq<c_t, d_t>();
}


TEST_F(message_type, compare)
{
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);

  EXPECT_EQ(b, a);
  EXPECT_NE(b, c);
  EXPECT_NE(b, d);

  EXPECT_NE(c, a);
  EXPECT_NE(c, b);
  EXPECT_EQ(c, d);

  EXPECT_NE(d, a);
  EXPECT_NE(d, b);
  EXPECT_EQ(d, c);

  // expected compilation failures
  // EXPECT_EQ(a, e);
  // EXPECT_EQ(e, a);
}


TEST_F(message_type, compare_type)
{
  // Note: there is no particular reason why comparing against x.type()
  // instead of uint16_t value beside silencing MSVC precision loss warning

  EXPECT_EQ(a.type(), a);
  EXPECT_EQ(a, a.type());
  EXPECT_NE(c.type(), a);
  EXPECT_NE(a, c.type());

  EXPECT_EQ(b.type(), b);
  EXPECT_EQ(b, b.type());
  EXPECT_NE(c.type(), b);
  EXPECT_NE(b, c.type());

  EXPECT_EQ(c.type(), c);
  EXPECT_EQ(c, c.type());
  EXPECT_NE(a.type(), c);
  EXPECT_NE(c, a.type());

  EXPECT_EQ(d.type(), d);
  EXPECT_EQ(d, d.type());
  EXPECT_NE(a.type(), d);
  EXPECT_NE(d, a.type());

  EXPECT_EQ(e.type(), e);
  EXPECT_EQ(e, e.type());
  EXPECT_NE(c.type(), e);
  EXPECT_NE(e, c.type());
}


TEST_F(message_type, ostream)
{
  std::ostringstream oss;
  oss << a;
  EXPECT_EQ(a_name, oss.str());

  oss.str("");
  oss << b;
  EXPECT_EQ(b_name, oss.str());

  oss.str("");
  oss << c;
  std::string expected = c_t::protocol_t::name();
  expected += ':';
  expected += std::to_string(c.type());
  EXPECT_EQ(expected, oss.str());

  oss.str("");
  oss << e;
  EXPECT_EQ(std::to_string(e.type()), oss.str());
}


TEST_F(message_type, name)
{
  EXPECT_STREQ(a_name, a.name());
  EXPECT_STREQ(b_name, b.name());
  EXPECT_STREQ(nullptr, c.name());
  EXPECT_STREQ(nullptr, d.name());
  EXPECT_STREQ(nullptr, e.name());
}


}} // namespace turner_test
