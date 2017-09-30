#pragma once

/**
 * \file turner/message.hpp
 */


#include <turner/config.hpp>
#include <turner/error.hpp>
#include <sal/assert.hpp>
#include <array>


__turner_begin


namespace __bits {

__turner_inline_var constexpr uint16_t
  class_mask = 0b0000'0001'0001'0000,
  indication_class = 0b0000'0000'0001'0000,
  success_response_class = 0b0000'0001'0000'0000,
  error_response_class = 0b0000'0001'0001'0000;

} // namespace __bits


using message_type_t = uint16_t;


/**
 * Generic request \t Method
 */
template <message_type_t Method>
__turner_inline_var constexpr message_type_t request_v = Method;


/**
 * Indication for \t Method
 */
template <message_type_t Method>
__turner_inline_var constexpr message_type_t indication_v =
  Method | __bits::indication_class;


/**
 * Success response for \t Method
 */
template <message_type_t Method>
__turner_inline_var constexpr message_type_t success_response_v =
  Method | __bits::success_response_class;


/**
 * Error response for \t Method
 */
template <message_type_t Method>
__turner_inline_var constexpr message_type_t error_response_v =
  Method | __bits::error_response_class;


/**
 */
class message_base_t
{
public:

  /**
   */
  message_base_t () = default;


  /**
   */
  bool is_valid () const noexcept
  {
    return begin_ != end_;
  }


  /**
   */
  explicit operator bool () const noexcept
  {
    return is_valid();
  }


  /**
   */
  message_type_t type () const noexcept;


  /**
   */
  uint16_t length () const noexcept;


  /**
   */
  bool is_indication () const noexcept
  {
    sal_assert(is_valid());
    return has_class(__bits::indication_class);
  }


  /**
   */
  bool is_success_response () const noexcept
  {
    sal_assert(is_valid());
    return has_class(__bits::success_response_class);
  }


  /**
   */
  bool is_error_response () const noexcept
  {
    sal_assert(is_valid());
    return has_class(__bits::error_response_class);
  }


  /**
   */
  size_t size () const noexcept
  {
    return static_cast<uint16_t>(end_ - begin_);
  }


protected:

  const uint8_t *begin_{}, *end_{};

  message_base_t (const uint8_t *first, const uint8_t *last) noexcept
    : begin_(first)
    , end_(last)
  {}

  bool has_class (uint16_t c) const noexcept
  {
    return (type() & c) == c;
  }
};


/**
 */
inline bool operator== (const message_base_t &message, message_type_t type)
  noexcept
{
  return message.type() == type;
}


/**
 */
inline bool operator== (message_type_t type, const message_base_t &message)
  noexcept
{
  return message.type() == type;
}


/**
 */
inline bool operator!= (const message_base_t &message, message_type_t type)
  noexcept
{
  return message.type() != type;
}


/**
 */
inline bool operator!= (message_type_t type, const message_base_t &message)
  noexcept
{
  return message.type() != type;
}


/**
 */
template <typename Protocol>
class basic_message_t
  : public message_base_t
{
public:

  /**
   */
  using protocol_t = Protocol;

  /**
   */
  using transaction_id_t = std::array<uint8_t, protocol_t::transaction_id_size>;


  /**
   */
  basic_message_t () = default;


  /**
   */
  template <typename It>
  static basic_message_t make (It first, It last, std::error_code &error)
    noexcept;


  /**
   */
  const transaction_id_t &transaction_id () const noexcept
  {
    sal_assert(is_valid());
    return *reinterpret_cast<const transaction_id_t *>(
      begin_ + protocol_t::transaction_id_offset
    );
  }


protected:

  basic_message_t (const uint8_t *first, const uint8_t *last) noexcept
    : message_base_t(first, last)
  {}
};


template <typename Protocol>
template <typename It>
basic_message_t<Protocol> basic_message_t<Protocol>::make (It first, It last,
  std::error_code &error) noexcept
{
  auto begin = reinterpret_cast<const uint8_t *>(std::addressof(*first));
  auto end = begin + (last - first) * sizeof(*first);

  // validate arguments
  if (!begin || !end || begin > end)
  {
    error = make_error_code(std::errc::invalid_argument);
    return {};
  }
  if (static_cast<size_t>(end - begin) < Protocol::header_size)
  {
    error = make_error_code(errc::insufficient_data);
    return {};
  }

  basic_message_t message(begin, end);

  // message type
  if (!Protocol::is_valid_message_type(message.type()))
  {
    error = make_error_code(errc::invalid_message_header);
    return {};
  }

  // message length (must be padded to 4B boundary)
  if ((message.length() & 0b11) != 0)
  {
    error = make_error_code(errc::invalid_message_length);
    return {};
  }
  if (Protocol::header_size + message.length() > message.size())
  {
    error = make_error_code(errc::insufficient_data);
    return {};
  }

  // cookie
  auto cookie = *reinterpret_cast<const decltype(Protocol::cookie) *>(
    begin + Protocol::cookie_offset
  );
  if (cookie != Protocol::cookie)
  {
    error = make_error_code(errc::invalid_message_header);
    return {};
  }

  // success
  error.clear();
  return message;
}


__turner_end
