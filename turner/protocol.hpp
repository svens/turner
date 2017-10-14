#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>
#include <memory>


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
      reinterpret_cast<const uint16_t *>(to_ptr(message))[0]
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
      reinterpret_cast<const uint16_t *>(to_ptr(message))[1]
    );
  }


private:

  template <typename It>
  static constexpr const uint8_t *to_ptr (It it) noexcept
  {
    return reinterpret_cast<const uint8_t *>(std::addressof(*it));
  }
};


/**
 * Generalised protocol description class.
 *
 * Specific Protocol defines own trait and wrap it into protocol_t which
 * provides common API for message parsing.
 */
template <typename Protocol, const char *Name>
class protocol_t
  : public protocol_base_t
{
public:

  /**
   * Return \a Name of the \a Protocol.
   */
  static constexpr const char *name () noexcept
  {
    return Name;
  }


  /**
   * Return instance of \a MessageType belonging to \a Protocol (with optional
   * \a MessageName)
   */
  template <uint16_t MessageType, const char *MessageName = nullptr>
  static constexpr const message_type_t<Protocol, MessageType, MessageName>
    message_type () noexcept
  {
    static_assert(
      (MessageType & __bits::class_mask) == 0,
      "not a valid message type"
    );
    return {};
  }


  /**
   * Write to \a stream \a Protocol \a Name. If \a Name is not specified,
   * nothing is written.
   */
  friend std::ostream &operator<< (std::ostream &stream, const protocol_t &)
  {
    if constexpr (Name != nullptr)
    {
      stream << Name;
    }
    return stream;
  }
};


__turner_end
