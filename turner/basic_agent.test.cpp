#include <turner/common.test.hpp>
#include <turner/basic_agent.hpp>
#include <turner/transport/mock.test.hpp>


namespace turner_test { namespace {


template <typename Protocol>
struct agent
  : public turner_test::with_protocol<Protocol>
{
  using traits_t = typename Protocol::traits_t;

  template <bool IsStream>
  using agent_t = turner::basic_agent_t<
    Protocol,
    ::testing::NiceMock<transport_mock_t<IsStream>>
  >;

  template <bool IsStream>
  struct client_t
    : public agent_t<IsStream>
  {
    // make protected members visible for testing
    using agent_t<IsStream>::receive;
    using agent_t<IsStream>::send;
    using agent_t<IsStream>::transport_;
  };

  using datagram_client_t = client_t<false>;
  using stream_client_t = client_t<true>;


  template <typename Data>
  Data add_framing_header (Data data)
  {
    if constexpr (Protocol::has_stream_framing_header)
    {
      using framing_header_t = typename traits_t::stream_framing_header_t;
      std::vector<uint8_t> buf(framing_header_t::size);
      new(buf.data()) framing_header_t(
        framing_header_t::type_t::control_message,
        data.size()
      );
      data.insert(data.begin(), buf.begin(), buf.end());
    }
    return data;
  }


  void SetUp () override
  {
    ::testing::DefaultValue<bool>::Set(true);
  }


  void TearDown () override
  {
    ::testing::DefaultValue<bool>::Clear();
  }
};

TYPED_TEST_CASE(agent, protocol_types);


using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgReferee;


TYPED_TEST(agent, datagram_receive)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed(TypeParam::message_data);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, datagram_receive_unexpected_message)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed(TypeParam::message_data);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::success_message, error));
  EXPECT_EQ(turner::errc::unexpected_message_type, error);
}


TYPED_TEST(agent, datagram_receive_with_integrity)
{
  typename TestFixture::datagram_client_t client;

  uint8_t buf[1024];
  auto hmac = TypeParam::message_hmac();
  auto [begin, end] = TypeParam::message.make(buf).finish(hmac);
  transport_feed_t feed(std::vector<uint8_t>{begin, end});

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, hmac, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, datagram_receive_with_invalid_integrity)
{
  typename TestFixture::datagram_client_t client;

  uint8_t buf[1024];
  auto hmac = TypeParam::message_hmac();
  auto [begin, end] = TypeParam::message.make(buf).finish(hmac);
  transport_feed_t feed(std::vector<uint8_t>{begin, end});

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  auto invalid_hmac = TypeParam::message_invalid_hmac();
  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::message, invalid_hmac, error));
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
}


TYPED_TEST(agent, datagram_receive_with_missing_integrity)
{
  typename TestFixture::datagram_client_t client;

  uint8_t buf[1024];
  auto [begin, end] = TypeParam::message.make(buf).finish();
  transport_feed_t feed(std::vector<uint8_t>{begin, end});

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  auto hmac = TypeParam::message_hmac();
  EXPECT_FALSE(client.receive(TypeParam::message, hmac, error));
  EXPECT_EQ(turner::errc::attribute_not_found, error);
}


TYPED_TEST(agent, datagram_receive_coalesced_message)
{
  typename TestFixture::datagram_client_t client;

  // first message
  auto data = TypeParam::message_data;

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::success_message.make(buf).finish();
  data.insert(data.end(), begin, end);

  transport_feed_t feed(data);
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, datagram_receive_chunked_message)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed(TypeParam::message_data);
  feed.chunk_size = 1;

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::message, error));
  EXPECT_FALSE(!error);
}


TYPED_TEST(agent, datagram_receive_full_and_partial_message)
{
  typename TestFixture::datagram_client_t client;

  // first message
  auto data = TypeParam::message_data;
  auto chunk_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::success_message.make(buf).finish();
  data.insert(data.end(), begin, end);
  chunk_size += (end - begin) / 2;

  transport_feed_t feed(data);
  feed.chunk_size = chunk_size;
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);

  EXPECT_FALSE(client.receive(TypeParam::success_message, error));
  EXPECT_FALSE(!error);
}


TYPED_TEST(agent, datagram_receive_two_messages)
{
  typename TestFixture::datagram_client_t client;

  // first message
  auto data = TypeParam::message_data;
  auto first_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::success_message.make(buf).finish();
  data.insert(data.end(), begin, end);

  transport_feed_t feed(data);
  feed.chunk_size = first_size;
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);

  EXPECT_TRUE(client.receive(TypeParam::success_message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, datagram_receive_invalid_data)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed(TestFixture::case_name);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::message, error));
  EXPECT_FALSE(!error);
}


TYPED_TEST(agent, datagram_receive_error)
{
  typename TestFixture::datagram_client_t client;

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(0)
      )
    );

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::not_enough_memory, error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, datagram_set_timeout)
{
  typename TestFixture::datagram_client_t client;

  using namespace std::chrono;
  using namespace std::chrono_literals;

  auto timeout = 1s;
  client.timeout(timeout);
  EXPECT_EQ(timeout, client.timeout());

  auto timeout_ms = duration_cast<milliseconds>(timeout);
  EXPECT_CALL(client.transport_, wait_receive_for(timeout_ms, _))
    .WillOnce(Return(false));

  std::error_code error;
  client.receive(error);
}


TYPED_TEST(agent, datagram_wait_timeout)
{
  typename TestFixture::datagram_client_t client;

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(false));

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .Times(0);

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::timed_out, error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, datagram_wait_error)
{
  typename TestFixture::datagram_client_t client;

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<1>(std::make_error_code(std::errc::not_enough_memory)),
        Return(0)
      )
    );

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .Times(0);

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::not_enough_memory, error);
  EXPECT_EQ(nullptr, message);
}




TYPED_TEST(agent, datagram_send)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed;

  // send
  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::send));

  std::error_code error;
  EXPECT_TRUE(
    client.send(TypeParam::message, error,
      TypeParam::username.value_cref(TestFixture::case_name)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(feed.data.empty());

  // receive and check
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::string_view username;
  EXPECT_TRUE(
    client.receive(TypeParam::message, error,
      TypeParam::username.value_ref(username)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(TestFixture::case_name, username);
}


TYPED_TEST(agent, datagram_send_with_integrity)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed;
  auto hmac = TypeParam::message_hmac();

  // send
  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::send));

  std::error_code error;
  EXPECT_TRUE(
    client.send(TypeParam::message, hmac, error,
      TypeParam::username.value_cref(TestFixture::case_name)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(feed.data.empty());

  // receive and check
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::string_view username;
  EXPECT_TRUE(
    client.receive(TypeParam::message, hmac, error,
      TypeParam::username.value_ref(username)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(TestFixture::case_name, username);
}


TYPED_TEST(agent, datagram_send_overflow)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed;

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::send));

  auto big_data = std::string(4000, 'X');
  std::error_code error;
  EXPECT_FALSE(
    client.send(TypeParam::message, error,
      TypeParam::username.value_cref(big_data)
    )
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(feed.data.empty());
}


TYPED_TEST(agent, datagram_send_with_integrity_overflow)
{
  typename TestFixture::datagram_client_t client;
  transport_feed_t feed;
  auto hmac = TypeParam::message_hmac();

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::send));

  auto big_data = std::string(4000, 'X');
  std::error_code error;
  EXPECT_FALSE(
    client.send(TypeParam::message, hmac, error,
      TypeParam::username.value_cref(big_data)
    )
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(feed.data.empty());
}


TYPED_TEST(agent, datagram_send_error)
{
  typename TestFixture::datagram_client_t client;

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(false)
      )
    );

  std::error_code error;
  EXPECT_FALSE(client.send(TypeParam::message, error));
  EXPECT_EQ(std::errc::not_enough_memory, error);
}


TYPED_TEST(agent, datagram_send_with_integrity_error)
{
  typename TestFixture::datagram_client_t client;
  auto hmac = TypeParam::message_hmac();

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(false)
      )
    );

  std::error_code error;
  EXPECT_FALSE(client.send(TypeParam::message, hmac, error));
  EXPECT_EQ(std::errc::not_enough_memory, error);
}


TYPED_TEST(agent, stream_receive)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed(TestFixture::add_framing_header(TypeParam::message_data));

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, stream_receive_unexpected_type)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed(TestFixture::add_framing_header(TypeParam::message_data));

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::success_message, error));
  EXPECT_EQ(turner::errc::unexpected_message_type, error);
}


TYPED_TEST(agent, stream_receive_with_integrity)
{
  typename TestFixture::stream_client_t client;

  uint8_t buf[1024];
  auto hmac = TypeParam::message_hmac();
  auto [begin, end] = TypeParam::message.make(buf).finish(hmac);
  transport_feed_t feed(
    TestFixture::add_framing_header(std::vector<uint8_t>{begin, end})
  );

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, hmac, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, stream_receive_with_invalid_integrity)
{
  typename TestFixture::stream_client_t client;

  uint8_t buf[1024];
  auto hmac = TypeParam::message_hmac();
  auto [begin, end] = TypeParam::message.make(buf).finish(hmac);
  transport_feed_t feed(
    TestFixture::add_framing_header(std::vector<uint8_t>{begin, end})
  );

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  auto invalid_hmac = TypeParam::message_invalid_hmac();
  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::message, invalid_hmac, error));
  EXPECT_EQ(turner::errc::unexpected_attribute_value, error);
}


TYPED_TEST(agent, stream_receive_with_missing_integrity)
{
  typename TestFixture::stream_client_t client;

  uint8_t buf[1024];
  auto [begin, end] = TypeParam::message.make(buf).finish();
  transport_feed_t feed(
    TestFixture::add_framing_header(std::vector<uint8_t>{begin, end})
  );

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  auto hmac = TypeParam::message_hmac();
  EXPECT_FALSE(client.receive(TypeParam::message, hmac, error));
  EXPECT_EQ(turner::errc::attribute_not_found, error);
}


TYPED_TEST(agent, stream_receive_coalesced_message)
{
  typename TestFixture::stream_client_t client;

  // first message
  auto data = TestFixture::add_framing_header(TypeParam::message_data);

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::success_message.make(buf).finish();
  auto second = TestFixture::add_framing_header(std::vector<uint8_t>(begin, end));
  data.insert(data.end(), second.begin(), second.end());

  transport_feed_t feed(data);
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillRepeatedly(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);

  EXPECT_TRUE(client.receive(TypeParam::success_message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, stream_receive_chunked_message)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed(TestFixture::add_framing_header(TypeParam::message_data));
  feed.chunk_size = 1;

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, stream_receive_full_and_partial_message)
{
  typename TestFixture::stream_client_t client;

  // first message
  auto data = TestFixture::add_framing_header(TypeParam::message_data);
  auto chunk_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::success_message.make(buf).finish();
  auto second = TestFixture::add_framing_header(std::vector<uint8_t>(begin, end));
  data.insert(data.end(), second.begin(), second.end());
  chunk_size += second.size() / 2;

  transport_feed_t feed(data);
  feed.chunk_size = chunk_size;
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);

  EXPECT_TRUE(client.receive(TypeParam::success_message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, stream_receive_two_messages)
{
  typename TestFixture::stream_client_t client;

  // first message
  auto data = TestFixture::add_framing_header(TypeParam::message_data);
  auto first_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::success_message.make(buf).finish();
  auto second = TestFixture::add_framing_header(std::vector<uint8_t>(begin, end));
  data.insert(data.end(), second.begin(), second.end());

  transport_feed_t feed(data);
  feed.chunk_size = first_size;
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_TRUE(client.receive(TypeParam::message, error));
  EXPECT_TRUE(!error);

  EXPECT_TRUE(client.receive(TypeParam::success_message, error));
  EXPECT_TRUE(!error);
}


TYPED_TEST(agent, stream_receive_invalid_data)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed(TestFixture::case_name);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  EXPECT_FALSE(client.receive(TypeParam::message, error));
  EXPECT_FALSE(!error);
}


TYPED_TEST(agent, stream_receive_error)
{
  typename TestFixture::stream_client_t client;

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(0)
      )
    );

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::not_enough_memory, error);
  EXPECT_EQ(nullptr, message);
}


inline std::string range_to_string (
  std::chrono::milliseconds a,
  std::chrono::milliseconds b)
{
  return "between "
    + std::to_string(a.count())
    + ".."
    + std::to_string(b.count())
    + "ms";
}


MATCHER_P2(Timeout, a, b, range_to_string(a, b))
{
  *result_listener << arg.count() << "ms";
  return a <= arg && arg <= b;
}


TYPED_TEST(agent, stream_set_timeout)
{
  typename TestFixture::stream_client_t client;

  using namespace std::chrono;
  using namespace std::chrono_literals;

  auto timeout = 1000ms;
  client.timeout(timeout);
  EXPECT_EQ(timeout, client.timeout());

  auto low  = timeout - 100ms;
  auto high  = timeout + 100ms;
  EXPECT_CALL(client.transport_, wait_receive_for(Timeout(low, high), _))
    .WillOnce(Return(false));

  std::error_code error;
  client.receive(error);
}


TYPED_TEST(agent, stream_receive_wait_timeout)
{
  typename TestFixture::stream_client_t client;

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(Return(false));

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .Times(0);

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::timed_out, error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, stream_receive_wait_error)
{
  typename TestFixture::stream_client_t client;

  ON_CALL(client.transport_, wait_receive_for(_, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<1>(std::make_error_code(std::errc::not_enough_memory)),
        Return(0)
      )
    );

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .Times(0);

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::not_enough_memory, error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, stream_receive_buffer_overflow)
{
  typename TestFixture::stream_client_t client;

  // Using internal implementation knowledge: client keeps buffer
  // as uint8_t[2000]. Create message bigger than that
  uint8_t buf[3 * sizeof(client)];
  auto [begin, end] = TypeParam::message.make(buf)
    .write(TypeParam::username, std::string(2 * sizeof(client), 'X'))
    .finish();
  std::vector<uint8_t> data(begin, end);

  transport_feed_t feed(TestFixture::add_framing_header(data));
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, stream_send)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed;

  // send
  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::send));

  std::error_code error;
  EXPECT_TRUE(
    client.send(TypeParam::message, error,
      TypeParam::username.value_cref(TestFixture::case_name)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(feed.data.empty());

  // receive and check
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::string_view username;
  EXPECT_TRUE(
    client.receive(TypeParam::message, error,
      TypeParam::username.value_ref(username)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(TestFixture::case_name, username);
}


TYPED_TEST(agent, stream_send_with_integrity)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed;
  auto hmac = TypeParam::message_hmac();

  // send
  EXPECT_CALL(client.transport_, send(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::send));

  std::error_code error;
  EXPECT_TRUE(
    client.send(TypeParam::message, hmac, error,
      TypeParam::username.value_cref(TestFixture::case_name)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  ASSERT_FALSE(feed.data.empty());

  // receive and check
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &transport_feed_t::receive));

  std::string_view username;
  EXPECT_TRUE(
    client.receive(TypeParam::message, hmac, error,
      TypeParam::username.value_ref(username)
    )
  );
  ASSERT_TRUE(!error) << error.message();
  EXPECT_EQ(TestFixture::case_name, username);
}


TYPED_TEST(agent, stream_send_overflow)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed;

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::send));

  auto big_data = std::string(4000, 'X');
  std::error_code error;
  EXPECT_FALSE(
    client.send(TypeParam::message, error,
      TypeParam::username.value_cref(big_data)
    )
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(feed.data.empty());
}


TYPED_TEST(agent, stream_send_with_integrity_overflow)
{
  typename TestFixture::stream_client_t client;
  transport_feed_t feed;
  auto hmac = TypeParam::message_hmac();

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(Invoke(&feed, &transport_feed_t::send));

  auto big_data = std::string(4000, 'X');
  std::error_code error;
  EXPECT_FALSE(
    client.send(TypeParam::message, hmac, error,
      TypeParam::username.value_cref(big_data)
    )
  );
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_TRUE(feed.data.empty());
}


TYPED_TEST(agent, stream_send_error)
{
  typename TestFixture::stream_client_t client;

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(false)
      )
    );

  std::error_code error;
  EXPECT_FALSE(client.send(TypeParam::message, error));
  EXPECT_EQ(std::errc::not_enough_memory, error);
}


TYPED_TEST(agent, stream_send_with_integrity_error)
{
  typename TestFixture::stream_client_t client;
  auto hmac = TypeParam::message_hmac();

  // send
  ON_CALL(client.transport_, send(_, _, _))
    .WillByDefault(
      DoAll(
        SetArgReferee<2>(std::make_error_code(std::errc::not_enough_memory)),
        Return(false)
      )
    );

  std::error_code error;
  EXPECT_FALSE(client.send(TypeParam::message, hmac, error));
  EXPECT_EQ(std::errc::not_enough_memory, error);
}


TYPED_TEST(agent, stream_connection_reset)
{
  typename TestFixture::stream_client_t client;

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Return(0));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(std::errc::connection_reset, error);
  EXPECT_EQ(nullptr, message);
}


}} // namespace turner_test
