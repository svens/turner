#pragma once

/**
 * \file turner/message.hpp
 */


#include <turner/config.hpp>
#include <sal/assert.hpp>
#include <array>


__turner_begin


namespace __bits {

inline constexpr uint16_t
  indication_class = 0b0000'0000'0001'0000,
  success_response_class = 0b0000'0001'0000'0000,
  error_response_class = 0b0000'0001'0001'0000;

} // namespace __bits


using message_type_t = uint16_t;


/**
 * Generic request \t Method
 */
template <message_type_t Method>
inline constexpr message_type_t method_v = Method;


/**
 * Indication for \t Method
 */
template <message_type_t Method>
inline constexpr message_type_t indication_v =
  Method | __bits::indication_class;


/**
 * Success response for \t Method
 */
template <message_type_t Method>
inline constexpr message_type_t success_response_v =
  Method | __bits::success_response_class;


/**
 * Error response for \t Method
 */
template <message_type_t Method>
inline constexpr message_type_t error_response_v =
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
  message_type_t type () const noexcept
  {
    return 1;
  }


  /**
   */
  bool is_indication () const noexcept
  {
    return has_class(__bits::indication_class);
  }


  /**
   */
  bool is_success_response () const noexcept
  {
    return has_class(__bits::success_response_class);
  }


  /**
   */
  bool is_error_response () const noexcept
  {
    return has_class(__bits::error_response_class);
  }


  /**
   */
  uint16_t length () const noexcept
  {
    return static_cast<uint16_t>(end_ - begin_);
  }


//protected:

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
  const transaction_id_t &transaction_id () const noexcept
  {
    sal_assert(is_valid());
    return *reinterpret_cast<const transaction_id_t *>(
      begin_ + protocol_t::transaction_id_offset
    );
  }


//protected:

  basic_message_t (const uint8_t *first, const uint8_t *last) noexcept
    : message_base_t(first, last)
  {}
};


__turner_end
