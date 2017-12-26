#pragma once

/**
 * \file turner/turn/protocol.hpp
 * TURN protocol (https://tools.ietf.org/html/rfc5766)
 */


#include <turner/config.hpp>
#include <turner/stun/protocol.hpp>


__turner_begin


namespace turn {


// Pull STUN functions into turn namespace
using stun::make_integrity_calculator;


/**
 * TURN protocol message layout traits. As TURN is extension of STUN, it is
 * inherited from it's layout.
 */
struct protocol_traits_t
  : public stun::protocol_traits_t
{};


/**
 * Structure type describing TURN protocol message layout.
 */
using protocol_t = turner::protocol_t<protocol_traits_t>;


/**
 * \copydoc turner::protocol_t::parse(It, It, std::error_code &);
 */
template <typename It>
inline const any_message_t<protocol_traits_t> *parse (It first, It last,
  std::error_code &error) noexcept
{
  return protocol_t::parse(first, last, error);
}


/**
 * \copydoc turner::protocol_t::parse(It, It);
 */
template <typename It>
inline const any_message_t<protocol_traits_t> *parse (It first, It last)
{
  return protocol_t::parse(first, last);
}


/**
 * \copydoc turner::protocol_t::parse(const Data &, std::error_code &);
 */
template <typename Data>
inline const any_message_t<protocol_traits_t> *parse (const Data &data,
  std::error_code &error) noexcept
{
  return protocol_t::parse(data, error);
}


/**
 * \copydoc turner::protocol_t::parse(const Data &);
 */
template <typename Data>
inline const any_message_t<protocol_traits_t> *parse (const Data &data)
{
  return protocol_t::parse(data);
}


} // namespace turn


__turner_end
