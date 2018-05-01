#pragma once

/**
 * \file turner/basic_agent.hpp
 * Common agent base functionality
 */

#include <turner/config.hpp>
#include <turner/agent_base.hpp>
#include <turner/error.hpp>
#include <turner/message_type.hpp>
#include <sal/time.hpp>


__turner_begin


/**
 * Agent base class with common functionality for \a Protocol and
 * \a Transport.
 *
 * Depending whether \a Transport::is_stream, this class has different
 * implementations for receiving messages from remote side:
 * - false: method receive() returns only full messages. Partial messages are
 *   ignored. In case of coalesced multiple messages, only first is returned
 *   and rest of received data is ignored.
 * - true: method receive() can be fed in chunks, it keeps gathering data into
 *   internal buffer until full message can be parsed and returned. In case of
 *   coalesced messages, it returns those sequentially until internal buffer
 *   is exhausted and transport's receive method is invoked again to fill
 *   buffer.
 */
template <typename Protocol, typename Transport>
class basic_agent_t
  : public agent_base_t
{
public:

  /**
   * Agent's protocol.
   */
  using protocol_t = Protocol;

  /**
   * Protocol traits.
   */
  using protocol_traits_t = typename Protocol::traits_t;

  /**
   * Agent's transport (independent from Protocol).
   */
  using transport_t = Transport;

  /**
   * True if agent's transport is stream oriented.
   */
  static constexpr bool is_stream_oriented = transport_t::is_stream;


  /**
   * Set timeout for receive operations.
   */
  template <typename Rep, typename Period>
  basic_agent_t &timeout (std::chrono::duration<Rep, Period> timeout) noexcept
  {
    timeout_ = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
    return *this;
  }


protected:

  /// \{
  transport_t transport_;
  std::chrono::milliseconds timeout_{3000};
  uint8_t buffer_[2000];
  uint8_t *end_ = buffer_, *last_message_end_ = end_;
  uint8_t * const buffer_end = buffer_ + sizeof(buffer_);
  /// \}


  /**
   * Construct new agent and it's transport.
   */
  template <typename... Arg>
  basic_agent_t (Arg &&...arg)
    : transport_(std::forward<Arg>(arg)...)
  {}


  basic_agent_t (const basic_agent_t &) = delete;
  basic_agent_t &operator= (const basic_agent_t &) = delete;


  /**
   * Receive next full message from remote side or nullptr on \a error.
   * In case of stream oriented transport, this method can return nullptr
   * without error when buffer has valid partial message.
   *
   * If stream oriented transport returns zero bytes, connection is assumed to
   * be reset by remote side (error is set to std::errc::connection_reset).
   */
  const any_message_t<protocol_traits_t> *receive (std::error_code &error)
    noexcept
  {
    error.clear();

    //
    // Stream (keep reading until full message, error or timeout)
    //
    if constexpr (Transport::is_stream)
    {
      const auto deadline = sal::now() + timeout_;
      do
      {
        if (auto message = from_buffer(error);  message || error)
        {
          return message;
        }
      } while (can_receive(deadline, error) && receive_into_buffer(error));
    }

    //
    // Datagram (read once)
    //
    else
    {
      if (transport_.wait_receive_for(timeout_, error))
      {
        if (auto size = transport_.receive(buffer_, buffer_end, error))
        {
          return Protocol::parse(buffer_, buffer_ + size, error);
        }
      }
    }

    if (!error)
    {
      error = std::make_error_code(std::errc::timed_out);
    }

    return {};
  }


private:

  const any_message_t<protocol_traits_t> *from_buffer (std::error_code &error)
    noexcept
  {
    if (end_ != last_message_end_)
    {
      auto begin = last_message_end_;

      if constexpr (protocol_t::has_stream_framing_header)
      {
        using framing_header_t = typename protocol_traits_t::stream_framing_header_t;

        size_t size = end_ - begin;
        if (size < framing_header_t::size)
        {
          return {};
        }

        auto framing_header = new(begin) framing_header_t;
        if (framing_header->type() != framing_header_t::type_t::control_message)
        {
          error = make_error_code(errc::invalid_message_type);
          return {};
        }
        if (framing_header->size + framing_header->length() > sizeof(buffer_))
        {
          error = make_error_code(errc::not_enough_room);
          return {};
        }

        // don't check begin + length > end here:
        // next if() deals with buffer squeezing
        begin += framing_header_t::size;
      }

      if (auto message = Protocol::parse(begin, end_, error))
      {
        last_message_end_ = begin + message->size();
        return message;
      }
      else if (error == errc::insufficient_header_data
        || error == errc::insufficient_payload_data)
      {
        if (last_message_end_ != buffer_)
        {
          end_ = std::uninitialized_copy(last_message_end_, end_, buffer_);
          last_message_end_ = buffer_;
        }
        if (end_ < buffer_end)
        {
          error.clear();
        }
        else
        {
          error = make_error_code(errc::not_enough_room);
        }
      }
    }
    else if (end_ != buffer_)
    {
      end_ = last_message_end_ = buffer_;
    }
    return {};
  }


  bool can_receive (sal::time_t deadline, std::error_code &error) noexcept
  {
    auto now = sal::now();
    if (now < deadline)
    {
      return transport_.wait_receive_for(
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now),
        error
      );
    }
    return false;
  }


  bool receive_into_buffer (std::error_code &error) noexcept
  {
    if (auto size = transport_.receive(end_, buffer_end, error))
    {
      end_ += size;
      return true;
    }
    else if (!error)
    {
      error = std::make_error_code(std::errc::connection_reset);
    }
    return false;
  }
};


__turner_end
