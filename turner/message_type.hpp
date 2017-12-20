#pragma once

/**
 * \file turner/message_type.hpp
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>


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
   * Protocol that defines this message type.
   */
  using protocol_t = turner::protocol_t<ProtocolTraits>;


  /**
   * Message type numeric value (in host byte order)
   */
  static inline constexpr uint16_t type = MessageType;


  /**
   * Return true if this \a MessageType is same as \a OtherMessageType
   */
  template <uint16_t OtherMessageType>
  constexpr bool operator== (message_type_t<ProtocolTraits, OtherMessageType>)
    const noexcept
  {
    return MessageType == OtherMessageType;
  }


  /**
   * Return true if this \a MessageType is not same as \a OtherMessageType
   */
  template <uint16_t OtherMessageType>
  constexpr bool operator!= (message_type_t<ProtocolTraits, OtherMessageType>)
    const noexcept
  {
    return MessageType != OtherMessageType;
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
