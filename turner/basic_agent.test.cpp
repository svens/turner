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


struct feed_t
{
  std::vector<uint8_t> data;
  std::vector<uint8_t>::const_iterator ptr;
  size_t chunk_size;

  template <typename Data>
  feed_t (const Data &content, size_t chunk_size = 0)
    : data(std::cbegin(content), std::cend(content))
    , ptr(std::cbegin(data))
    , chunk_size(chunk_size)
  { }

  size_t receive (uint8_t *first, uint8_t *last, std::error_code &)
  {
    size_t size = (std::min)(last - first, data.end() - ptr);
    if (chunk_size && chunk_size < size)
    {
      size = chunk_size;
    }

    std::uninitialized_copy_n(ptr, size,
      sal::__bits::make_output_iterator(first, last)
    );
    ptr += size;

    return size;
  }
};


TYPED_TEST(agent, datagram_receive)
{
  typename TestFixture::datagram_client_t client;
  feed_t feed(TypeParam::msg_data());

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();

  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());
}


TYPED_TEST(agent, datagram_receive_coalesced_message)
{
  typename TestFixture::datagram_client_t client;

  // first message
  auto data = TypeParam::msg_data();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::msg_success_type().make(buf).finish();
  data.insert(data.end(), begin, end);

  feed_t feed(data);
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();

  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());
}


TYPED_TEST(agent, datagram_receive_chunked_message)
{
  typename TestFixture::datagram_client_t client;
  feed_t feed(TypeParam::msg_data(), 1);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_FALSE(!error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, datagram_receive_full_and_partial_message)
{
  typename TestFixture::datagram_client_t client;

  // first message
  auto data = TypeParam::msg_data();
  auto chunk_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::msg_success_type().make(buf).finish();
  data.insert(data.end(), begin, end);
  chunk_size += (end - begin) / 2;

  feed_t feed(data, chunk_size);
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());

  message = client.receive(error);
  EXPECT_FALSE(!error);
  EXPECT_EQ(nullptr, message);
}


TYPED_TEST(agent, datagram_receive_two_messages)
{
  typename TestFixture::datagram_client_t client;

  // first message
  auto data = TypeParam::msg_data();
  auto first_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::msg_success_type().make(buf).finish();
  data.insert(data.end(), begin, end);

  feed_t feed(data, first_size);
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());

  message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_success_type(), message->type());
}


TYPED_TEST(agent, datagram_receive_invalid_data)
{
  typename TestFixture::datagram_client_t client;
  feed_t feed(TestFixture::case_name);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_FALSE(!error);
  EXPECT_EQ(nullptr, message);
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


TYPED_TEST(agent, stream_receive)
{
  typename TestFixture::stream_client_t client;
  feed_t feed(TestFixture::add_framing_header(TypeParam::msg_data()));

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();

  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());
}


TYPED_TEST(agent, stream_receive_coalesced_message)
{
  typename TestFixture::stream_client_t client;

  // first message
  auto data = TestFixture::add_framing_header(TypeParam::msg_data());

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::msg_success_type().make(buf).finish();
  auto second = TestFixture::add_framing_header(std::vector<uint8_t>(begin, end));
  data.insert(data.end(), second.begin(), second.end());

  feed_t feed(data);
  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillRepeatedly(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());

  message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_success_type(), message->type());
}


TYPED_TEST(agent, stream_receive_chunked_message)
{
  typename TestFixture::stream_client_t client;
  feed_t feed(TestFixture::add_framing_header(TypeParam::msg_data()), 1);

  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());
}


TYPED_TEST(agent, stream_receive_full_and_partial_message)
{
  typename TestFixture::stream_client_t client;

  // first message
  auto data = TestFixture::add_framing_header(TypeParam::msg_data());
  auto chunk_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::msg_success_type().make(buf).finish();
  auto second = TestFixture::add_framing_header(std::vector<uint8_t>(begin, end));
  data.insert(data.end(), second.begin(), second.end());
  chunk_size += second.size() / 2;

  feed_t feed(data, chunk_size);
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());

  message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_success_type(), message->type());
}


TYPED_TEST(agent, stream_receive_two_messages)
{
  typename TestFixture::stream_client_t client;

  // first message
  auto data = TestFixture::add_framing_header(TypeParam::msg_data());
  auto first_size = data.size();

  // add second message
  uint8_t buf[1024];
  auto [begin, end] = TypeParam::msg_success_type().make(buf).finish();
  auto second = TestFixture::add_framing_header(std::vector<uint8_t>(begin, end));
  data.insert(data.end(), second.begin(), second.end());

  feed_t feed(data, first_size);
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_type(), message->type());

  message = client.receive(error);
  EXPECT_TRUE(!error) << error.message();
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(TypeParam::msg_success_type(), message->type());
}


TYPED_TEST(agent, stream_receive_invalid_data)
{
  typename TestFixture::stream_client_t client;
  feed_t feed(TestFixture::case_name);

  EXPECT_CALL(client.transport_, receive(_, _, _))
    .WillOnce(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_FALSE(!error);
  EXPECT_EQ(nullptr, message);
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


TYPED_TEST(agent, stream_receive_buffer_overflow)
{
  typename TestFixture::stream_client_t client;

  // Using internal implementation knowledge: client keeps buffer
  // as uint8_t[2000]. Create message bigger than that
  uint8_t buf[3 * sizeof(client)];
  auto [begin, end] = TypeParam::msg_type().make(buf)
    .write(TypeParam::attr_type(), std::string(2 * sizeof(client), 'X'))
    .finish();
  std::vector<uint8_t> data(begin, end);

  feed_t feed(TestFixture::add_framing_header(data));
  ON_CALL(client.transport_, receive(_, _, _))
    .WillByDefault(Invoke(&feed, &feed_t::receive));

  std::error_code error;
  auto message = client.receive(error);
  EXPECT_EQ(turner::errc::not_enough_room, error);
  EXPECT_EQ(nullptr, message);
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
