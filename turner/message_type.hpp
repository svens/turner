#pragma once

/**
 * \file turner/message_type.hpp
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <ostream>


__turner_begin


/**
 * Define \a Protocol message \a Type.
 *
 * \a Type must follow rules defined by RFC5389, section 6. To define
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
template <typename Protocol, uint16_t Type>
class basic_message_type_t
{
public:

  static_assert((Type & __bits::method_mask) == 0, "invalid message type");


  /**
   * Protocol that defines this message type.
   */
  using protocol_t = basic_protocol_t<Protocol>;


  /**
   * Return message type numeric value (in host byte order)
   */
  static constexpr uint16_t type () noexcept
  {
    return Type;
  }


  /**
   * Return message \a Type name (if defined).
   *
   * \a Type name is defined, if \c "void operator>> (Type, const char *&name)"
   * is provided. On operator invocation, name should be assigned to
   * \a Type name.
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
   * Write to \a stream message \a Type name.
   *
   * Depending on which name getter operators are provided, name is formatted
   * as follows:
   *  - name(): write returned string
   *  - if no name() but have protocol_t::name(): write as "Protocol:Type"
   *  - if no name() and no protocol_t::name(): write as "Type"
   */
  friend std::ostream &operator<< (std::ostream &stream, basic_message_type_t)
  {
    if constexpr (name() != nullptr)
    {
      return (stream << name());
    }
    else if constexpr (protocol_t::name() != nullptr)
    {
      return (stream << protocol_t::name() << ':' << Type);
    }
    else
    {
      return (stream << Type);
    }
  }


  /**
   * Success response for message \a Type.
   */
  using success_response_t = basic_message_type_t<Protocol,
    Type | __bits::success_response_class
  >;


  /**
   * Return instance of message \a Type success response.
   */
  static constexpr success_response_t success_response () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Error response for message \a Type.
   */
  using error_response_t = basic_message_type_t<Protocol,
    Type | __bits::error_response_class
  >;


  /**
   * Return instance of message \a Type error response.
   */
  static constexpr error_response_t error_response () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Indication for message \a Type.
   */
  using indication_t = basic_message_type_t<Protocol,
    Type | __bits::indication_class
  >;


  /**
   * Return instance of message \a Type indication.
   */
  static constexpr indication_t indication () noexcept
  {
    expect_request_class();
    return {};
  }


  /**
   * Return true if this message \a Type is same as \a OtherType
   */
  template <uint16_t OtherType>
  constexpr bool operator== (basic_message_type_t<Protocol, OtherType>)
    const noexcept
  {
    return Type == OtherType;
  }


  /**
   * Return true if this message \a Type is not same as \a OtherType
   */
  template <uint16_t OtherType>
  constexpr bool operator!= (basic_message_type_t<Protocol, OtherType>)
    const noexcept
  {
    return Type != OtherType;
  }


private:

  static constexpr void expect_request_class () noexcept
  {
    static_assert((Type & __bits::class_mask) == 0, "expected request class");
  }

  friend class basic_protocol_t<Protocol>;
};


/**
 * Return true if message \a Type is same as numeric \a type.
 */
template <typename Protocol, uint16_t Type>
inline constexpr bool operator== (basic_message_type_t<Protocol, Type>,
  uint16_t type) noexcept
{
  return Type == type;
}


/**
 * Return true if message \a Type is same as numeric \a type.
 */
template <typename Protocol, uint16_t Type>
inline constexpr bool operator== (uint16_t type,
  basic_message_type_t<Protocol, Type>) noexcept
{
  return Type == type;
}


/**
 * Return true if message \a Type is not same as numeric \a type.
 */
template <typename Protocol, uint16_t Type>
inline constexpr bool operator!= (basic_message_type_t<Protocol, Type>,
  uint16_t type) noexcept
{
  return Type != type;
}


/**
 * Return true if message \a Type is not same as numeric \a type.
 */
template <typename Protocol, uint16_t Type>
inline constexpr bool operator!= (uint16_t type,
  basic_message_type_t<Protocol, Type>) noexcept
{
  return Type != type;
}


__turner_end
