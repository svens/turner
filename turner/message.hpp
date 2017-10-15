#pragma once

/**
 * \file turner/message.hpp
 * Protocol message
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <sal/byte_order.hpp>
#include <type_traits>
#include <utility>


__turner_begin


class message_base_t
{
public:

  constexpr message_base_t () = default;

  message_base_t (const message_base_t &) = delete;
  message_base_t &operator= (const message_base_t &) = delete;


  message_base_t (message_base_t &&that) noexcept
    : begin_(that.begin_)
    , end_(that.end_)
  {
    that.begin_ = that.end_ = nullptr;
  }


  message_base_t &operator= (message_base_t &&that) noexcept
  {
    auto tmp = std::move(that);
    swap(tmp);
    return *this;
  }


  void swap (message_base_t &that) noexcept
  {
    using std::swap;
    swap(begin_, that.begin_);
    swap(end_, that.end_);
  }


  constexpr bool is_valid () const noexcept
  {
    return begin_ != end_;
  }


  constexpr explicit operator bool () const noexcept
  {
    return is_valid();
  }


protected:

  const uint8_t *begin_{};
  const uint8_t *end_{};

  constexpr message_base_t (const uint8_t *begin, const uint8_t *end) noexcept
    : begin_(begin)
    , end_(end)
  {}
};


template <typename Protocol>
class any_message_t
  : public message_base_t
{
public:

  using protocol_t = basic_protocol_t<Protocol>;
  using cookie_t = typename protocol_t::cookie_t;


  any_message_t () = default;


  constexpr uint16_t type () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(begin_)[0]
    );
  }


  constexpr uint16_t length () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(begin_)[1]
    );
  }


  constexpr const cookie_t &cookie () const noexcept
  {
    return *reinterpret_cast<const cookie_t *>(
      begin_ + Protocol::cookie_offset()
    );
  }


private:

  constexpr any_message_t (const uint8_t *begin, const uint8_t *end) noexcept
    : message_base_t(begin, end)
  {}

  friend class basic_protocol_t<Protocol>;
};


__turner_end
