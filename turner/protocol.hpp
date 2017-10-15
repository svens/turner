#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>


__turner_begin


/**
 * Common base class for protocol classes.
 */
class protocol_base_t
{
protected:

  /**
   * Return \a message type.
   *
   * \note This method intentionally does not check length of message. This is
   * responsibility of message parsing.
   */
  template <typename It>
  static constexpr uint16_t type (It message) noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(__bits::to_ptr(message))[0]
    );
  }


  /**
   * Return \a message length.
   *
   * \note This method intentionally does not check length of message. This is
   * responsibility of message parsing.
   */
  template <typename It>
  static constexpr uint16_t length (It message) noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(__bits::to_ptr(message))[1]
    );
  }
};


/**
 * Generalised protocol description class.
 *
 * Specific Protocol defines own trait and wrap it into basic_protocol_t which
 * provides common API for message parsing.
 */
template <typename Protocol>
class basic_protocol_t
  : public protocol_base_t
{
public:

  /**
   * Write to \a stream \a Protocol name. If name is not defined, nothing is
   * written.
   */
  friend std::ostream &operator<< (std::ostream &stream, basic_protocol_t)
  {
    if constexpr (__bits::has_name_v<Protocol>)
    {
      stream << Protocol::name();
    }
    return stream;
  }
};


__turner_end
