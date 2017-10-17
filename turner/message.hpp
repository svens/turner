#pragma once

/**
 * \file turner/message.hpp
 * Protocol message
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>


__turner_begin


/**
 * Generic \a Protocol message, not tied to any message type.
 *
 * It can be used to parse STUN-based protocols only as it's header layout
 * depends on RFC5389, section 6.
 *
 * This type is not meant to be instantiated directly but from
 * basic_protocol_t<Protocol>::from_wire() that does message validation. On
 * success, returned pointed object does not have any own storage. It is
 * overlayed on top of memory range containing raw network formatted message
 * and all getter methods operate on fields offset relative from this and
 * described by \a Protocol.
 */
template <typename Protocol>
class any_message_t
{
public:

  /**
   * Protocol class describing raw network message format.
   */
  using protocol_t = basic_protocol_t<Protocol>;


  /**
   * Per protocol unique cookie type. It is additional helper to detect
   * network message protocol if multiple protocols are multiplexed on single
   * channel.
   */
  using cookie_t = typename protocol_t::cookie_t;


  /**
   * Per message unique transaction ID.
   */
  using transaction_id_t = typename protocol_t::transaction_id_t;


  /**
   * Return message type code in native byte order. Code values are defined by
   * protocol documentation.
   */
  constexpr uint16_t type () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(this)[0]
    );
  }


  /**
   * Return message size as claimed by message length field. It does not
   * include message header length nor possible padding.
   */
  constexpr uint16_t length () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(this)[1]
    );
  }


  /**
   * Return message cookie.
   */
  constexpr const cookie_t &cookie () const noexcept
  {
    return *reinterpret_cast<const cookie_t *>(
      __bits::to_ptr(this) + Protocol::cookie_offset()
    );
  }


  /**
   * Return message transaction ID.
   */
  constexpr const transaction_id_t &transaction_id () const noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      __bits::to_ptr(this) + Protocol::transaction_id_offset()
    );
  }


  /**
   * Return specialized message from \a this generic message if underlying raw
   * message has expected \a Type. On different type, return nullptr.
   */
  template <uint16_t Type>
  constexpr const basic_message_t<Protocol, Type> *
    as (message_type_t<Protocol, Type>) const noexcept
  {
    return type() == Type
      ? reinterpret_cast<const basic_message_t<Protocol, Type> *>(this)
      : nullptr;
  }


private:

  any_message_t () = delete;
};


/**
 * Specialized \a Protocol message of \a Type.
 */
template <typename Protocol, uint16_t Type>
class basic_message_t
  : public any_message_t<Protocol>
{
public:

  /**
   * message_type_t instance for this message \a Type.
   */
  static __turner_inline_var constexpr const message_type_t<Protocol, Type>
    message_type{};
};


__turner_end
