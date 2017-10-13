#pragma once

/**
 * \file turner/message_type.hpp
 * Protocol message type
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <ostream>


__turner_begin


/**
 * Define \a Protocol message \a Type (with optional \a Name)
 */
template <typename Protocol, uint16_t Type, const char *Name = nullptr>
class basic_message_type_t
{
public:

  /**
   * Protocol which defines this message type.
   */
  using protocol_t = Protocol;


  /**
   * Return \a Name of the message type.
   */
  static constexpr const char *name () noexcept
  {
    return Name;
  }


  /**
   * Return message type numeric value (in host byte order).
   */
  static constexpr uint16_t type () noexcept
  {
    return Type;
  }


  /**
   * Return true if this message \a Type is same as \a OtherType.
   */
  template <uint16_t OtherType, const char *OtherName>
  constexpr bool operator== (basic_message_type_t<Protocol, OtherType, OtherName>)
    const noexcept
  {
    return Type == OtherType;
  }


  /**
   * Return true if this message \a Type is not same as \a OtherType.
   */
  template <uint16_t OtherType, const char *OtherName>
  constexpr bool operator!= (basic_message_type_t<Protocol, OtherType, OtherName>)
    const noexcept
  {
    return Type != OtherType;
  }


  /**
   * Write to \a stream message type \a Name. If \a Name is not specified,
   * write numeric type() instead (with optional \a Protocol name if it is not
   * nullptr).
   */
  friend std::ostream &operator<< (std::ostream &stream, basic_message_type_t)
  {
    if constexpr (Name != nullptr)
    {
      stream << Name;
    }
    else if constexpr (Protocol::name() != nullptr)
    {
      stream << Protocol::name() << ':' << Type;
    }
    else
    {
      stream << Type;
    }
    return stream;
  }
};


/**
 * Return true if message \a Type is same as numeric \a type.
 */
template <typename Protocol, uint16_t Type, const char *Name>
inline constexpr bool operator== (basic_message_type_t<Protocol, Type, Name>,
  uint16_t type) noexcept
{
  return Type == type;
}


/**
 * Return true if message \a Type is same as numeric \a type.
 */
template <typename Protocol, uint16_t Type, const char *Name>
inline constexpr bool operator== (uint16_t type,
  basic_message_type_t<Protocol, Type, Name>) noexcept
{
  return Type == type;
}


/**
 * Return true if message \a Type is not same as numeric \a type.
 */
template <typename Protocol, uint16_t Type, const char *Name>
inline constexpr bool operator!= (basic_message_type_t<Protocol, Type, Name>,
  uint16_t type) noexcept
{
  return Type != type;
}


/**
 * Return true if message \a Type is not same as numeric \a type.
 */
template <typename Protocol, uint16_t Type, const char *Name>
inline constexpr bool operator!= (uint16_t type,
  basic_message_type_t<Protocol, Type, Name>) noexcept
{
  return Type != type;
}


__turner_end
