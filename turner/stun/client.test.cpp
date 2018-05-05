#include <turner/common.test.hpp>
#include <turner/transport/mock.test.hpp>


namespace turner_test { namespace {


namespace stun = turner::stun;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgReferee;


template <typename Transport>
struct stun_client
  : public turner_test::with_type<Transport>
{
  struct client_t
    : public stun::client_t<Transport>
  {
    using stun::client_t<Transport>::transport_;
  };
};

using datagram = ::testing::NiceMock<transport_mock_t<false>>;
using stream = ::testing::NiceMock<transport_mock_t<true>>;
using transport_types = ::testing::Types<datagram, stream>;
TYPED_TEST_CASE(stun_client, transport_types);


auto address_v4 = sal::net::ip::make_address("1.2.3.4");
auto address_v6 = sal::net::ip::make_address("::ffff:1.2.3.4");
uint16_t port = 5678;


auto binding_success (const sal::net::ip::address_t &address)
{
  uint8_t buf[1024];
  auto [first, last] = stun::binding_success.make(buf)
    .write(stun::xor_mapped_address, {address, port})
    .finish();
  return std::vector<uint8_t>{first, last};
}


auto binding_without_address ()
{
  uint8_t buf[1024];
  auto [first, last] = stun::binding_success.make(buf).finish();
  return std::vector<uint8_t>{first, last};
}


auto binding_failure ()
{
  uint8_t buf[1024];
  auto [first, last] = STUN::msg_error_type().make(buf).finish();
  return std::vector<uint8_t>{first, last};
}


void expect_stun_binding (const uint8_t *first, const uint8_t *last,
  ::testing::Unused)
{
  std::error_code error;
  auto message = stun::parse(first, last, error);
  EXPECT_TRUE(!error);
  ASSERT_TRUE(message);
  EXPECT_EQ(stun::binding, message->type());
}


TYPED_TEST(stun_client, binding_v4)
{
  typename TestFixture::client_t client;

  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(
      DoAll(
        Invoke(expect_stun_binding),
        Return(true)
      )
    );

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(binding_success(address_v4));
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  auto [result_address, result_port] = client.binding(error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(address_v4, result_address);
  EXPECT_EQ(port, result_port);
}


TYPED_TEST(stun_client, binding_v6)
{
  typename TestFixture::client_t client;

  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(
      DoAll(
        Invoke(expect_stun_binding),
        Return(true)
      )
    );

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(binding_success(address_v6));
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  auto [result_address, result_port] = client.binding(error);
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(address_v6, result_address);
  EXPECT_EQ(port, result_port);
}


TYPED_TEST(stun_client, binding_send_fails)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(false)
      )
    );

  std::error_code error;
  client.binding(error);
  EXPECT_EQ(std::errc::not_enough_memory, error);

  EXPECT_THROW(
    client.binding(),
    std::system_error
  );
}


TYPED_TEST(stun_client, binding_receive_fails)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(0)
      )
    );

  std::error_code error;
  client.binding(error);
  EXPECT_EQ(std::errc::not_enough_memory, error);
}


TYPED_TEST(stun_client, binding_receive_time_out)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(false));

  std::error_code error;
  client.binding(error);
  EXPECT_EQ(std::errc::timed_out, error);
}


TYPED_TEST(stun_client, binding_invalid)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(TestFixture::case_name);
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  client.binding(error);
  EXPECT_FALSE(!error);
}


TYPED_TEST(stun_client, binding_unexpected_message_type)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(binding_failure());
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  client.binding(error);
  EXPECT_EQ(turner::errc::unexpected_message_type, error);
}


TYPED_TEST(stun_client, binding_attribute_not_found)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(binding_without_address());
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  client.binding(error);
  EXPECT_EQ(turner::errc::attribute_not_found, error);
}


TYPED_TEST(stun_client, binding_chunked)
{
  typename TestFixture::client_t client;

  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Return(true));

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(binding_success(address_v4));
  feed.chunk_size = 1;

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillRepeatedly(Invoke(&feed, &transport_feed_t::receive));

  if constexpr (TypeParam::is_stream)
  {
    std::error_code error;
    auto [result_address, result_port] = client.binding(error);
    ASSERT_TRUE(!error) << error.message();
    EXPECT_EQ(address_v4, result_address);
    EXPECT_EQ(port, result_port);
  }
  else
  {
    std::error_code error;
    client.binding(error);
    EXPECT_FALSE(!error);
  }
}


TYPED_TEST(stun_client, server_reflexive_address)
{
  typename TestFixture::client_t client;

  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(
      DoAll(
        Invoke(expect_stun_binding),
        Return(true)
      )
    );

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(true));

  transport_feed_t feed(binding_success(address_v4));
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  for (int i = 0;  i < 3;  ++i)
  {
    std::error_code error;
    auto [result_address, result_port] = client.server_reflexive_address(error);
    ASSERT_TRUE(!error) << error.message();
    EXPECT_EQ(address_v4, result_address);
    EXPECT_EQ(port, result_port);
  }
}


}} // namespace turner_test
