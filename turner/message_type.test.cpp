#include <turner/common.test.hpp>
#include <turner/message_type.hpp>
#include <turner/protocol.hpp>


namespace turner_test { namespace {


using message_type = turner_test::fixture;


TEST_F(message_type, protocol)
{
  EXPECT_EQ(
    typeid(msg_1_t::protocol_t),
    typeid(turner::basic_protocol_t<turner_test::protocol_t>)
  );

  EXPECT_EQ(
    typeid(msg_1_t::protocol_t),
    typeid(msg_1a_t::protocol_t)
  );

  EXPECT_EQ(
    typeid(msg_1_t::protocol_t),
    typeid(msg_2_t::protocol_t)
  );

  EXPECT_NE(
    typeid(msg_1_t::protocol_t),
    typeid(up_msg_1_t::protocol_t)
  );
}


TEST_F(message_type, type)
{
  EXPECT_EQ(uint16_t(1), msg_1.type());
  EXPECT_EQ(uint16_t(1), msg_1a.type());
  EXPECT_EQ(uint16_t(2), msg_2.type());
  EXPECT_EQ(uint16_t(1), up_msg_1.type());
}


TEST_F(message_type, name)
{
  const char *name;
  msg_1 >> name;
  EXPECT_STREQ(name, msg_1.name());
}


TEST_F(message_type, name_unnamed)
{
  EXPECT_EQ(nullptr, msg_2.name());
  EXPECT_EQ(nullptr, up_msg_1.name());
}


TEST_F(message_type, ostream)
{
  const char *name;
  msg_1 >> name;

  std::ostringstream oss;
  oss << msg_1;
  EXPECT_EQ(name, oss.str());
}


TEST_F(message_type, ostream_unnamed_with_named_protocol)
{
  std::ostringstream oss, expected;
  expected << msg_1_t::protocol_t{} << ':' << msg_2.type();
  oss << msg_2;
  EXPECT_EQ(expected.str(), oss.str());
}


TEST_F(message_type, ostream_unnamed_with_unnamed_protocol)
{
  std::ostringstream oss, expected;
  expected << up_msg_1.type();
  oss << up_msg_1;
  EXPECT_EQ(expected.str(), oss.str());
}


TEST_F(message_type, compare)
{
  EXPECT_EQ(msg_1, msg_1a);
  EXPECT_NE(msg_1, msg_2);

  // expected compilation error:
  // (void)(msg_1 == up_msg_1);
  // (void)(msg_1 != up_msg_1);
}


TEST_F(message_type, compare_int)
{
  EXPECT_EQ(uint16_t(1), msg_1);
  EXPECT_EQ(msg_1, uint16_t(1));
  EXPECT_NE(uint16_t(2), msg_1);
  EXPECT_NE(msg_1, uint16_t(2));

  EXPECT_EQ(uint16_t(1), msg_1a);
  EXPECT_EQ(msg_1a, uint16_t(1));
  EXPECT_NE(uint16_t(2), msg_1a);
  EXPECT_NE(msg_1a, uint16_t(2));

  EXPECT_EQ(uint16_t(2), msg_2);
  EXPECT_EQ(msg_2, uint16_t(2));
  EXPECT_NE(uint16_t(1), msg_2);
  EXPECT_NE(msg_2, uint16_t(1));
}


TEST_F(message_type, success_response)
{
  constexpr const auto msg_1_ok = msg_1.success_response();
  EXPECT_NE(msg_1, msg_1_ok);

  EXPECT_EQ(
    msg_1.type() | turner::__bits::success_response_class,
    msg_1_ok.type()
  );

  EXPECT_EQ(
    typeid(msg_1_t::success_response_t),
    typeid(msg_1_ok)
  );

  EXPECT_EQ(
    typeid(decltype(msg_1_ok)::protocol_t),
    typeid(turner::basic_protocol_t<turner_test::protocol_t>)
  );
}


TEST_F(message_type, error_response)
{
  constexpr const auto msg_1_fail = msg_1.error_response();
  EXPECT_NE(msg_1, msg_1_fail);

  EXPECT_EQ(
    msg_1.type() | turner::__bits::error_response_class,
    msg_1_fail.type()
  );

  EXPECT_EQ(
    typeid(msg_1_t::error_response_t),
    typeid(msg_1_fail)
  );

  EXPECT_EQ(
    typeid(decltype(msg_1_fail)::protocol_t),
    typeid(turner::basic_protocol_t<turner_test::protocol_t>)
  );
}


TEST_F(message_type, indication)
{
  constexpr const auto msg_1_ind = msg_1.indication();
  EXPECT_NE(msg_1, msg_1_ind);

  EXPECT_EQ(
    msg_1.type() | turner::__bits::indication_class,
    msg_1_ind.type()
  );

  EXPECT_EQ(
    typeid(msg_1_t::indication_t),
    typeid(msg_1_ind)
  );

  EXPECT_EQ(
    typeid(decltype(msg_1_ind)::protocol_t),
    typeid(turner::basic_protocol_t<turner_test::protocol_t>)
  );
}


}} // namespace turner_test
