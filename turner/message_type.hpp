#pragma once

/**
 * \file turner/message_type.hpp
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/crypto/random.hpp>
#include <sal/memory.hpp>
#include <iterator>


__turner_begin


/**
 * Return true if message \a type is valid according to STUN RFC
 * (two most significant bits should be zero)
 */
inline constexpr bool is_valid_message_type (uint16_t type) noexcept
{
  return (type & 0b1100'0000'0000'0000) == 0;
}


/**
 * Define protocol \a MessageType.
 */
template <typename ProtocolTraits, uint16_t MessageType>
class message_type_t
{
public:

  static_assert(is_valid_message_type(MessageType), "invalid message type");


  /**
   * Protocol traits.
   */
  using traits_t = ProtocolTraits;


  /**
   * Protocol that defines this message type.
   */
  using protocol_t = turner::protocol_t<traits_t>;


  /**
   * Message type numeric value (in host byte order)
   */
  static inline constexpr const uint16_t type = MessageType;


  /**
   * Return true if this \a MessageType is same as \a OtherMessageType
   */
  template <uint16_t OtherMessageType>
  constexpr bool operator== (message_type_t<traits_t, OtherMessageType>)
    const noexcept
  {
    return MessageType == OtherMessageType;
  }


  /**
   * Return true if this \a MessageType is not same as \a OtherMessageType
   */
  template <uint16_t OtherMessageType>
  constexpr bool operator!= (message_type_t<traits_t, OtherMessageType>)
    const noexcept
  {
    return MessageType != OtherMessageType;
  }


  /**
   * Try to cast generic \a message to concrete \a type message.
   * \returns pointer to concrete message on success or nullptr on failure.
   */
  const message_reader_t<traits_t, MessageType> *operator() (
    const any_message_t<traits_t> *message) const noexcept
  {
    return message->type() == type
      ? reinterpret_cast<const message_reader_t<traits_t, type> *>(message)
      : nullptr;
  }


  /**
   * Return message writer object for this message type. Returned object is
   * allowed to write attributes into memory area [\a first, \a last).
   *
   * This method builds immediately message header but it does not add any
   * attributes. Use returned object to add attributes and finalize message.
   *
   * On error, set \a error and return undefined message_writer_t object.
   */
  template <typename It>
  message_writer_t<traits_t, MessageType> make (It first, It last,
    std::error_code &error) const noexcept
  {
    if (first == last)
    {
      error = make_error_code(errc::not_enough_room);
      return {nullptr, nullptr};
    }

    auto begin = sal::to_ptr(first);
    auto end = sal::to_end_ptr(first, last);
    if (begin + protocol_t::header_and_cookie_size() <= end)
    {
      reinterpret_cast<uint16_t *>(begin)[0] =
        sal::native_to_network_byte_order(MessageType);

      reinterpret_cast<uint16_t *>(begin)[1] =
        sal::native_to_network_byte_order(protocol_t::min_payload_length());

      *reinterpret_cast<typename protocol_t::cookie_t *>(
        begin + traits_t::cookie_offset
      ) = traits_t::cookie;

      sal::crypto::random(
        begin + traits_t::transaction_id_offset,
        begin + traits_t::transaction_id_offset + traits_t::transaction_id_size
      );

      error.clear();
      return {begin, end};
    }

    error = make_error_code(errc::not_enough_room);
    return {nullptr, nullptr};
  }


  /**
   * Return message writer object for \a MessageType. Returned object is
   * allowed to write attributes into memory area [\a first, \a last).
   *
   * This method builds immediately message header but it does not add any
   * attributes. Use returned object to add attributes and finalize message.
   *
   * \throws std::system_error on message header building failure.
   */
  template <typename It>
  message_writer_t<traits_t, MessageType> make (It first, It last) const
  {
    return make(first, last, sal::throw_on_error("message_type::make"));
  }


  /**
   * Return message writer object for \a MessageType. Returned object is
   * allowed to write attributes into memory area \a data.
   *
   * This method builds immediately message header but it does not add any
   * attributes. Use returned object to add attributes and finalize message.
   *
   * On error, set \a error and return undefined message_writer_t object.
   */
  template <typename Data>
  message_writer_t<traits_t, MessageType> make (Data &data, std::error_code &error)
    const noexcept
  {
    using std::begin;
    using std::end;
    return make(begin(data), end(data), error);
  }


  /**
   * Return message writer object for \a MessageType. Returned object is
   * allowed to write attributes into memory area \a data.
   *
   * This method builds immediately message header but it does not add any
   * attributes. Use returned object to add attributes and finalize message.
   *
   * \throws std::system_error on message header building failure.
   */
  template <typename Data>
  message_writer_t<traits_t, MessageType> make (Data &data) const
  {
    return make(data, sal::throw_on_error("message_type::make"));
  }
};


/**
 * Return true if \a MessageType is same as numeric \a type.
 */
template <typename ProtocolTraits, uint16_t MessageType>
inline constexpr bool operator== (
  message_type_t<ProtocolTraits, MessageType>, uint16_t type) noexcept
{
  return MessageType == type;
}


/**
 * Return true if \a MessageType is same as numeric \a type.
 */
template <typename ProtocolTraits, uint16_t MessageType>
inline constexpr bool operator== (
  uint16_t type, message_type_t<ProtocolTraits, MessageType>) noexcept
{
  return MessageType == type;
}


/**
 * Return true if \a MessageType is not same as numeric \a type.
 */
template <typename ProtocolTraits, uint16_t MessageType>
inline constexpr bool operator!= (
  message_type_t<ProtocolTraits, MessageType>, uint16_t type) noexcept
{
  return MessageType != type;
}


/**
 * Return true if \a MessageType is not same as numeric \a type.
 */
template <typename ProtocolTraits, uint16_t MessageType>
inline constexpr bool operator!= (
  uint16_t type, message_type_t<ProtocolTraits, MessageType>) noexcept
{
  return MessageType != type;
}


__turner_end
