#pragma once

/**
 * \file turner/message_type.hpp
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <ostream>


__turner_begin


/**
 * Define \a Protocol \a Message.
 *
 * \a Message value must follow rules defined by RFC5389, section 6. To define
 * message's responses, there are conveniency typedefs:
 *  - basic_message_type_t::success_response_t
 *  - basic_message_type_t::error_response_t
 *  - basic_message_type_t::indication_t
 *
 * Although application code might need to know specific types, it is more
 * convenient to use instances of basic_message_type_t. To create such
 * instances, there are static methods:
 *  - protocol_t::message_type() to create message type that has no class
 *  - basic_message_type_t::success_response() to create message type instance
 *    corresponding to this message type success response
 *  - basic_message_type_t::error_response() to create message type instance
 *    corresponding to this message type error response
 *  - basic_message_type_t::indication() to create message type instance
 *    corresponding to this message type indication
 */
template <typename Protocol, uint16_t Message>
class basic_message_type_t
{
public:

  static_assert((Message & __bits::method_mask) == 0, "invalid message type");


  /**
   * Protocol that defines this message type.
   */
  using protocol_t = basic_protocol_t<Protocol>;


  /**
   * Return message type numeric value (in host byte order)
   */
  static constexpr uint16_t type () noexcept
  {
    return Message;
  }


  /**
   * Return \a Message name (if defined).
   *
   * \a Message name is defined, if
   * \c "void operator>> (basic_message_type<Protocol, Message>, const char *&name)"
   * is provided. On operator invocation, name should be assigned to
   * \a Message name.
   *
   * If this operator is not provided, nullptr is returned.
   */
  static constexpr const char *name () noexcept
  {
    if constexpr (__bits::has_name_getter_v<basic_message_type_t>)
    {
      const char *result{};
      basic_message_type_t{} >> result;
      return result;
    }
    else
    {
      return nullptr;
    }
  }


  /**
   * Write to \a stream \a Message name.
   *
   * Depending on which name getter operators are provided, name is formatted
   * as follows:
   *  - name(): write returned string
   *  - if no name() but have protocol_t::name(): write as "Protocol:type"
   *  - if no name() and no protocol_t::name(): write as "type"
   */
  friend std::ostream &operator<< (std::ostream &stream, basic_message_type_t)
  {
    if constexpr (name() != nullptr)
    {
      return (stream << name());
    }
    else if constexpr (protocol_t::name() != nullptr)
    {
      return (stream << protocol_t::name() << ':' << Message);
    }
    else
    {
      return (stream << Message);
    }
  }


  /**
   * Success response for \a Message.
   */
  using success_response_t = basic_message_type_t<Protocol,
    Message | __bits::success_response_class
  >;


  /**
   * Return instance of \a Message success response.
   */
  static constexpr success_response_t success_response () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Error response for \a Message.
   */
  using error_response_t = basic_message_type_t<Protocol,
    Message | __bits::error_response_class
  >;


  /**
   * Return instance of \a Message error response.
   */
  static constexpr error_response_t error_response () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Indication for \a Message.
   */
  using indication_t = basic_message_type_t<Protocol,
    Message | __bits::indication_class
  >;


  /**
   * Return instance of \a Message indication.
   */
  static constexpr indication_t indication () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Return true if this \a Message is same as \a OtherMessage
   */
  template <uint16_t OtherMessage>
  constexpr bool operator== (basic_message_type_t<Protocol, OtherMessage>)
    const noexcept
  {
    return Message == OtherMessage;
  }


  /**
   * Return true if this \a Message is not same as \a OtherMessage
   */
  template <uint16_t OtherMessage>
  constexpr bool operator!= (basic_message_type_t<Protocol, OtherMessage>)
    const noexcept
  {
    return Message != OtherMessage;
  }


private:

  static constexpr void expect_request_class () noexcept
  {
    static_assert((Message & __bits::class_mask) == 0, "expected request class");
  }

  friend class basic_protocol_t<Protocol>;
};


/**
 * Return true if \a Message is same as numeric \a type.
 */
template <typename Protocol, uint16_t Message>
inline constexpr bool operator== (basic_message_type_t<Protocol, Message>,
  uint16_t type) noexcept
{
  return Message == type;
}


/**
 * Return true if \a Message is same as numeric \a type.
 */
template <typename Protocol, uint16_t Message>
inline constexpr bool operator== (uint16_t type,
  basic_message_type_t<Protocol, Message>) noexcept
{
  return Message == type;
}


/**
 * Return true if \a Message is not same as numeric \a type.
 */
template <typename Protocol, uint16_t Message>
inline constexpr bool operator!= (basic_message_type_t<Protocol, Message>,
  uint16_t type) noexcept
{
  return Message != type;
}


/**
 * Return true if \a Message is not same as numeric \a type.
 */
template <typename Protocol, uint16_t Message>
inline constexpr bool operator!= (uint16_t type,
  basic_message_type_t<Protocol, Message>) noexcept
{
  return Message != type;
}


__turner_end
