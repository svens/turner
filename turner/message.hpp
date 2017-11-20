#pragma once

/**
 * \file turner/message.hpp
 * Protocol message
 */

#include <turner/config.hpp>
#include <turner/error.hpp>
#include <turner/fwd.hpp>
#include <turner/__bits/helpers.hpp>
#include <turner/__bits/message.hpp>
#include <sal/byte_order.hpp>


__turner_begin


/**
 * Generic protocol message, not tied to any concrete message type.
 *
 * It can be used to parse STUN, TURN or MSTURN protocol messages only as it's
 * header layout depends on corresponding protocol header layout.
 *
 * This type is not meant to be instantiated directly but from
 * protocol_t<ProtocolTraits>::parse() that does message validation.
 * On success, returned pointed object does not have any own storage. It is
 * overlayed on top of memory range containing raw network formatted message
 * and all getter methods operate on fields offset relative from \a this and
 * described by \a ProtocolTraits.
 */
template <typename ProtocolTraits>
class any_message_t
{
public:

  /**
   * Protocol class describing raw network message format.
   */
  using protocol_t = turner::protocol_t<ProtocolTraits>;


  /**
   * Per protocol magic cookie type. It is additional helper to detect
   * network message protocol if multiple protocols are multiplexed on single
   * channel.
   */
  using cookie_t = typename protocol_t::cookie_t;


  /**
   * Per message unique transaction ID.
   */
  using transaction_id_t = typename protocol_t::transaction_id_t;


  /**
   * Return message type code in native byte order. Code values are defined by
   * concrete protocol documentation.
   */
  uint16_t type () const noexcept
  {
    return sal::network_to_native_byte_order(as_ptr<uint16_t>()[0]);
  }


  /**
   * Return true if message type is request (i.e. not response or indication).
   */
  bool is_request () const noexcept
  {
    return (type() & __bits::class_mask) == 0;
  }


  /**
   * Return true if message type is success response.
   */
  bool is_success_response () const noexcept
  {
    return (type() & __bits::class_mask) == __bits::success_response_class;
  }


  /**
   * Return true if message type is error response.
   */
  bool is_error_response () const noexcept
  {
    return (type() & __bits::class_mask) == __bits::error_response_class;
  }


  /**
   * Return true if message type is indication.
   */
  bool is_indication () const noexcept
  {
    return (type() & __bits::class_mask) == __bits::indication_class;
  }


  /**
   * Return message size as claimed by message length field. It does not
   * include message header length.
   */
  uint16_t length () const noexcept
  {
    return sal::network_to_native_byte_order(as_ptr<uint16_t>()[1]);
  }


  /**
   * Return message cookie.
   */
  const cookie_t &cookie () const noexcept
  {
    return *reinterpret_cast<const cookie_t *>(
      as_ptr<uint8_t>() + ProtocolTraits::cookie_offset
    );
  }


  /**
   * Return message transaction ID.
   */
  const transaction_id_t &transaction_id () const noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      as_ptr<uint8_t>() + ProtocolTraits::transaction_id_offset
    );
  }


  /**
   * Return concrete message instance from \a this generic message if
   * underlying raw data has expected \a MessageType. On different type,
   * return nullptr.
   */
  template <uint16_t MessageType>
  const message_reader_t<ProtocolTraits, MessageType> *try_as (
    message_type_t<ProtocolTraits, MessageType>) const noexcept
  {
    return type() == MessageType
      ? as_ptr<message_reader_t<ProtocolTraits, MessageType>>()
      : nullptr;
  }


  /**
   * Return concrete message instance from \a this generic message if
   * underlying raw data has expected \a MessageType.
   *
   * \throws std::system_error if \a this message type is not \a MessageType.
   */
  template <uint16_t MessageType>
  const message_reader_t<ProtocolTraits, MessageType> &as (
    message_type_t<ProtocolTraits, MessageType>) const
  {
    if (type() == MessageType)
    {
      return *as_ptr<message_reader_t<ProtocolTraits, MessageType>>();
    }
    unexpected_message_type("any_message::as");
  }


protected:

  /// \cond internal

  any_message_t () = delete;
  any_message_t (const any_message_t &) = delete;
  any_message_t &operator= (const any_message_t &) = delete;
  any_message_t (any_message_t &&) = delete;
  any_message_t &operator= (any_message_t &&) = delete;

  static void unexpected_message_type [[noreturn]] (const char msg[])
  {
    sal::throw_system_error(
      make_error_code(errc::unexpected_message_type),
      msg
    );
  }

  template <typename T>
  const T *as_ptr () const noexcept
  {
    return reinterpret_cast<const T *>(this);
  }

  /// \endcond
};


/**
 * Concrete protocol's message reader.
 */
template <typename ProtocolTraits, uint16_t MessageType>
class message_reader_t
  : public any_message_t<ProtocolTraits>
{
public:

  /**
   * Message type.
   */
  using message_type_t = turner::message_type_t<ProtocolTraits, MessageType>;


  /**
   * Return message type.
   */
  static constexpr message_type_t type () noexcept
  {
    return {};
  }


  /**
   * Decode and return \a AttributeType value from message. On failure set \a
   * error to code describing failure and return default value.
   */
  template <
    uint16_t AttributeType,
    template <typename> typename AttributeProcessor
  >
  typename AttributeProcessor<ProtocolTraits>::value_t read (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor>,
    std::error_code &error) const noexcept
  {
    auto p = this->template as_ptr<uint8_t>() + ProtocolTraits::header_size;
    if (auto attribute = __bits::find_attribute(p, p + this->length(),
        AttributeType, ProtocolTraits::padding_size, error))
    {
      return AttributeProcessor<ProtocolTraits>::read(*this, *attribute, error);
    }
    return {};
  }


  /**
   * Decode and return \a AttributeType value from message. On failure throw
   * \a std::system_error.
   */
  template <
    uint16_t AttributeType,
    template <typename> typename AttributeProcessor
  >
  typename AttributeProcessor<ProtocolTraits>::value_t read (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor> attribute) const
  {
    return read(attribute, sal::throw_on_error("message_reader::read"));
  }


  /**
   * Create success response for this message into region [\a first, \a last).
   * On failure, set \a error and returned object is in undefined state.
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object. In which case, after call to this method,
   * \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, MessageType | __bits::success_response_class>
    to_success_response (It first, It last, std::error_code &error) const noexcept
  {
    return to_response<MessageType | __bits::success_response_class>(
      first, last, error
    );
  }


  /**
   * Create success response for this message into region [\a first, \a last).
   * On failure, throw \c std::system_error
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object. In which case, after call to this method,
   * \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, MessageType | __bits::success_response_class>
    to_success_response (It first, It last) const
  {
    return to_success_response(first, last,
      sal::throw_on_error("message_reader::to_success_response")
    );
  }


  /**
   * Create error response for this message into region [\a first, \a last).
   * On failure, set \a error and returned object is in undefined state.
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object. In which case, after call to this method,
   * \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, MessageType | __bits::error_response_class>
    to_error_response (It first, It last, std::error_code &error) const noexcept
  {
    return to_response<MessageType | __bits::error_response_class>(
      first, last, error
    );
  }


  /**
   * Create error response for this message into region [\a first, \a last).
   * On failure, throw \c std::system_error
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object. In which case, after call to this method,
   * \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, MessageType | __bits::error_response_class>
    to_error_response (It first, It last) const
  {
    return to_error_response(first, last,
      sal::throw_on_error("message_reader::to_error_response")
    );
  }


private:

  template <uint16_t ResponseMessageType, typename It>
  message_writer_t<ProtocolTraits, ResponseMessageType> to_response (
    It first, It last, std::error_code &error) const noexcept
  {
    static_assert((MessageType & __bits::class_mask) == 0,
      "expected request message type"
    );

    auto begin = sal::to_ptr(first), end = sal::to_end_ptr(first, last);
    if (begin + ProtocolTraits::header_size <= end)
    {
      auto src = this->template as_ptr<uint8_t>();
      if (src != begin)
      {
        std::memmove(begin, src, ProtocolTraits::header_size);
      }

      reinterpret_cast<uint16_t *>(begin)[0] =
        sal::native_to_network_byte_order(ResponseMessageType);
      reinterpret_cast<uint16_t *>(begin)[1] = 0;

      error.clear();
      return { begin, end };
    }

    error = make_error_code(errc::not_enough_room);
    return { nullptr, nullptr };
  }
};


/**
 * Concrete protocol's message writer.
 */
template <typename ProtocolTraits, uint16_t MessageType>
class message_writer_t
{
public:

  /**
   * Protocol class describing raw network message format.
   */
  using protocol_t = turner::protocol_t<ProtocolTraits>;

  /**
   * Message type.
   */
  using message_type_t = turner::message_type_t<ProtocolTraits, MessageType>;


  /**
   * Per protocol magic cookie type. It is additional helper to detect
   * network message protocol if multiple protocols are multiplexed on single
   * channel.
   */
  using cookie_t = typename protocol_t::cookie_t;


  /**
   * Per message unique transaction ID.
   */
  using transaction_id_t = typename protocol_t::transaction_id_t;


  /**
   * Return message type.
   */
  static constexpr message_type_t type () noexcept
  {
    return {};
  }


  /**
   * Return true if this object is in valid state. Calling any other method is
   * undefined behavious if object is not in valid state.
   */
  explicit operator bool () const noexcept
  {
    return first_ + ProtocolTraits::header_size <= last_;
  }


  /**
   * Return message size as claimed by message length field. It does not
   * include message header length.
   */
  uint16_t length () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(first_)[1]
    );
  }


  /**
   * Return message cookie.
   */
  const cookie_t &cookie () const noexcept
  {
    return *reinterpret_cast<const cookie_t *>(
      sal::to_ptr(first_) + ProtocolTraits::cookie_offset
    );
  }


  /**
   * Return message transaction ID.
   */
  const transaction_id_t &transaction_id () const noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      sal::to_ptr(first_) + ProtocolTraits::transaction_id_offset
    );
  }


  /**
   * Return available room for building message (in bytes).
   */
  uint16_t available () const noexcept
  {
    return static_cast<uint16_t>(
      last_ - first_ - ProtocolTraits::header_size - length()
    );
  }


  /**
   * Encode \a value and append to message as \a AttributeType. If there is
   * no enough room in internal buffer, \a set error to errc::not_enough_room
   * and do nothing.
   */
  template <uint16_t AttributeType, template <typename> typename AttributeProcessor>
  message_writer_t &write (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor>,
    const typename AttributeProcessor<ProtocolTraits>::value_t &value,
    std::error_code &error
  ) noexcept;


  /**
   * Encode \a value and append to message as \a AttributeType. If there is
   * no enough room in internal buffer, throw std::system_error.
   */
  template <uint16_t AttributeType, template <typename> typename AttributeProcessor>
  message_writer_t &write (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor> attribute,
    const typename AttributeProcessor<ProtocolTraits>::value_t &value)
  {
    return write(attribute, value,
      sal::throw_on_error("message_writer::write")
    );
  }


private:

  uint8_t * const first_, * const last_;

  message_writer_t (uint8_t *first, uint8_t *last) noexcept
    : first_(first)
    , last_(last)
  {}

  friend class turner::protocol_t<ProtocolTraits>;
  friend class message_reader_t<ProtocolTraits, MessageType & ~__bits::class_mask>;
};


template <typename ProtocolTraits, uint16_t MessageType>
template <uint16_t AttributeType, template <typename> typename AttributeProcessor>
message_writer_t<ProtocolTraits, MessageType> &
  message_writer_t<ProtocolTraits, MessageType>::write (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor>,
    const typename AttributeProcessor<ProtocolTraits>::value_t &value,
    std::error_code &error) noexcept
{
  auto &message = *reinterpret_cast<any_message_t<ProtocolTraits> *>(first_);
  auto message_size = message.length();

  auto attribute = first_ + ProtocolTraits::header_size + message_size;
  auto attribute_size = AttributeProcessor<ProtocolTraits>::write(message,
    attribute + 2 * sizeof(uint16_t),
    last_,
    value,
    error
  );

  if (!error)
  {
    if constexpr (ProtocolTraits::padding_size > 1)
    {
      // AttributeProcessor does not check for padding, do it here
      constexpr const auto r = ProtocolTraits::padding_size - 1;
      message_size += 2 * sizeof(uint16_t) + ((attribute_size + r) & ~r);
      if (first_ + ProtocolTraits::header_size + message_size > last_)
      {
        error = make_error_code(errc::not_enough_room);
        return *this;
      }
    }

    // attribute type
    reinterpret_cast<uint16_t *>(attribute)[0] =
      sal::native_to_network_byte_order(AttributeType);

    // attribute length
    reinterpret_cast<uint16_t *>(attribute)[1] =
      sal::native_to_network_byte_order(static_cast<uint16_t>(attribute_size));

    // message length
    reinterpret_cast<uint16_t *>(first_)[1] =
      sal::native_to_network_byte_order(message_size);
  }

  return *this;
}


__turner_end
