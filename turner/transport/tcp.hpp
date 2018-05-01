#pragma once

/**
 * \file turner/transport/tcp.hpp
 * TCP transport class.
 */

#include <turner/config.hpp>
#include <sal/buf_ptr.hpp>
#include <sal/net/ip/tcp.hpp>


__turner_begin


namespace transport {


/**
 * Synchronous TCP socket transport. It is thin proxy for
 * sal::net::ip::tcp_t::socket_t unifying transport API for
 * turner::basic_agent_t
 */
class tcp_t
{
public:

  /// \{
  static constexpr bool is_stream = false;
  /// \}


  /**
   * Construct new connected TCP socket to \a address and \a port
   */
  tcp_t (const sal::net::ip::address_t &address, uint16_t port)
  {
    socket_.connect({address, port});
  }


  /**
   * Wait up to \a timeout milliseconds for incoming data.
   * \returns false if no data available
   */
  bool wait_receive_for (std::chrono::milliseconds timeout,
    std::error_code &error) noexcept
  {
    return socket_.wait(socket_.wait_read, timeout, error);
  }


  /**
   * Copy incoming bytes into [\a first, \a last).
   * \returns number of bytes received or zero on \a error or when remote side
   * closed connection.
   */
  size_t receive (uint8_t *first, uint8_t *last, std::error_code &error)
    noexcept
  {
    return socket_.receive(sal::make_buf(first, last - first), error);
  }


  /**
   * Send bytes in [\a first, \a last) to remote side.
   * \returns true on no \a error.
   */
  bool send (const uint8_t *first, const uint8_t *last, std::error_code &error)
    noexcept
  {
    socket_.send(sal::make_buf(first, last - first), error);
    return !error;
  }


private:

  sal::net::ip::tcp_t::socket_t socket_{};
};


} // namespace transport


__turner_end
