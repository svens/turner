#pragma once

/**
 * \file turner/message_type.hpp
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>


__turner_begin


/**
 * Define protocol \a MessageType.
 *
 * \a MessageType value must follow rules defined by RFC5389, section 6. To
 * define message's responses, there are conveniency typedefs:
 *  - message_type_t::success_response_t
 *  - message_type_t::error_response_t
 *  - message_type_t::indication_t
 *
 * Although application scope might need to know specific types, it is more
 * convenient to use instances of message_type_t. To create such
 * instances, there are static methods provided:
 *  - protocol_t::message_type() to create message type that has no request
 *    class
 *  - message_type_t::success_response() to create message type instance
 *    corresponding to this message type success response
 *  - message_type_t::error_response() to create message type instance
 *    corresponding to this message type error response
 *  - message_type_t::indication() to create message type instance
 *    corresponding to this message type indication
 *
 * \see https://tools.ietf.org/html/rfc5389#section-6
 */
template <typename ProtocolTraits, uint16_t MessageType>
class message_type_t
{
public:

  static_assert(ProtocolTraits::is_valid_message_type(MessageType),
    "invalid message type"
  );


  /**
   * Protocol that defines this message type.
   */
  using protocol_t = turner::protocol_t<ProtocolTraits>;


  /**
   * Return message type numeric value (in host byte order)
   */
  static constexpr uint16_t type () noexcept
  {
    return MessageType;
  }


  /**
   * Return true if MessageType is request (i.e. not response or indication).
   */
  static constexpr bool is_request () noexcept
  {
    return ProtocolTraits::is_request(MessageType);
  }


  /**
   * Return true if MessageType is success response.
   */
  static constexpr bool is_success_response () noexcept
  {
    return ProtocolTraits::is_success_response(MessageType);
  }


  /**
   * Return true if MessageType is error response.
   */
  static constexpr bool is_error_response () noexcept
  {
    return ProtocolTraits::is_error_response(MessageType);
  }


  /**
   * Return true if MessageType is indication.
   */
  static constexpr bool is_indication () noexcept
  {
    return ProtocolTraits::is_indication(MessageType);
  }


  /**
   * Success response for \a MessageType.
   */
  using success_response_t = message_type_t<ProtocolTraits,
    ProtocolTraits::to_success_response(MessageType)
  >;


  /**
   * Return instance of \a MessageType success response.
   */
  static constexpr success_response_t success_response () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Error response for \a MessageType.
   */
  using error_response_t = message_type_t<ProtocolTraits,
    ProtocolTraits::to_error_response(MessageType)
  >;


  /**
   * Return instance of \a MessageType error response.
   */
  static constexpr error_response_t error_response () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Indication for \a MessageType.
   */
  using indication_t = message_type_t<ProtocolTraits,
    ProtocolTraits::to_indication(MessageType)
  >;


  /**
   * Return instance of \a MessageType indication.
   */
  static constexpr indication_t indication () noexcept
  {
    expect_request_class();
    return {};
  }


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


private:

  static constexpr void expect_request_class () noexcept
  {
    static_assert(is_request(), "expected request class");
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
