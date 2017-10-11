#pragma once

/**
 * \file turner/protocol.hpp
 * Protocol description (header, cookie, transaction id, etc)
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>
#include <array>
#include <memory>


__turner_begin


/**
 * Common base class for protocol classes.
 */
class protocol_base_t
{};


/**
 * Generalised protocol description class.
 *
 * Specialised protocols define own Protocol traits class and wrap it into
 * basic_protocol_t which provides common API for message parsing.
 */
template <typename Protocol>
class basic_protocol_t
  : public protocol_base_t
{
public:

  /**
   * Underlying protocol traits.
   */
  using traits_t = Protocol;

  /**
   * Message cookie type
   */
  using cookie_t = std::array<uint8_t, traits_t::cookie_size>;

  /**
   * Message transaction ID type
   */
  using transaction_id_t = std::array<uint8_t, traits_t::transaction_id_size>;


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


  /**
   * Return cooke of the \a message.
   *
   * \note This method intentionally does not check length of message. This is
   * responsibility of message parsing.
   */
  template <typename It>
  static constexpr const cookie_t &cookie (It message) noexcept
  {
    return *reinterpret_cast<const cookie_t *>(
      to_ptr(message) + traits_t::cookie_offset
    );
  }


  /**
   * Return transaction ID of the \a message.
   *
   * \note This method intentionally does not check length of message. This is
   * responsibility of message parsing.
   */
  template <typename It>
  static constexpr const transaction_id_t &transaction_id (It message) noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      to_ptr(message) + traits_t::transaction_id_offset
    );
  }


private:

  template <typename It>
  static constexpr const uint8_t *to_ptr (It it) noexcept
  {
    return reinterpret_cast<const uint8_t *>(std::addressof(*it));
  }
};


__turner_end
