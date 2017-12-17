#pragma once

/**
 * \file turner/message.hpp
 * Protocol message
 */

#include <turner/config.hpp>
#include <turner/error.hpp>
#include <turner/fwd.hpp>
#include <turner/__bits/message.hpp>
#include <sal/byte_order.hpp>
#include <sal/crypto/hmac.hpp>


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
   * Return true if message type is request (ie not response or indication).
   */
  bool is_request () const noexcept
  {
    return ProtocolTraits::is_request(type());
  }


  /**
   * Return true if message type is success response.
   */
  bool is_success_response () const noexcept
  {
    return ProtocolTraits::is_success_response(type());
  }


  /**
   * Return true if message type is error response.
   */
  bool is_error_response () const noexcept
  {
    return ProtocolTraits::is_error_response(type());
  }


  /**
   * Return true if message type is indication.
   */
  bool is_indication () const noexcept
  {
    return ProtocolTraits::is_indication(type());
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


  /**
   * Return true if message's claimed integrity is same as calculated with \a
   * integrity_calculator. On failure, set \a error code identifying problem
   * and return false.
   *
   * Possible errors:
   * - turner::errc::attribute_not_found: message does not have
   *   MESSAGE-INTEGRITY attribute
   * - turner::errc::unexpected_attribute_length: invalid integrity length
   * - turner::errc::unexpected_attribute_value: claimed integrity is not
   *   valid
   */
  template <typename Digest>
  bool has_valid_integrity (sal::crypto::hmac_t<Digest> &integrity_calculator,
    std::error_code &error
  ) const noexcept;


  /**
   * \copybrief has_valid_integrity
   * On failure, throw std::system_error.
   */
  template <typename Digest>
  bool has_valid_integrity (sal::crypto::hmac_t<Digest> &integrity_calculator)
    const
  {
    return has_valid_integrity(integrity_calculator,
      sal::throw_on_error("any_message::has_valid_integrity")
    );
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
    auto payload = this->template as_ptr<uint8_t>() + header_size;
    if (auto attribute = __bits::find_attribute(payload,
        payload + this->length() - min_length,
        AttributeType,
        ProtocolTraits::padding_size,
        error))
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
   * occupied by this object in which case after call \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_success_response(MessageType)>
    to_success_response (It first, It last, std::error_code &error) const noexcept
  {
    return to_response<ProtocolTraits::to_success_response(MessageType)>(
      first, last, error
    );
  }


  /**
   * Create success response for this message into region [\a first, \a last).
   * On failure, throw \c std::system_error
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object in which case after call \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_success_response(MessageType)>
    to_success_response (It first, It last) const
  {
    return to_success_response(first, last,
      sal::throw_on_error("message_reader::to_success_response")
    );
  }


  /**
   * Create success response for this message into region \a data.
   * On failure, set \a error and returned object is in undefined state.
   *
   * Specified region \a data can overlap with memory area occupied by this
   * object in which case after call \a this becomes invalid.
   */
  template <typename Data>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_success_response(MessageType)>
    to_success_response (Data &data, std::error_code &error) const noexcept
  {
    using std::begin;
    using std::end;
    return to_response<ProtocolTraits::to_success_response(MessageType)>(
      begin(data), end(data), error
    );
  }


  /**
   * Create success response for this message into region \a data.
   * On failure, throw \c std::system_error
   *
   * Specified region \a data can overlap with memory area occupied by this
   * object in which case after call \a this becomes invalid.
   */
  template <typename Data>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_success_response(MessageType)>
    to_success_response (Data &data) const
  {
    return to_success_response(data,
      sal::throw_on_error("message_reader::to_success_response")
    );
  }


  /**
   * Create error response for this message into region [\a first, \a last).
   * On failure, set \a error and returned object is in undefined state.
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object in which case after call \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_error_response(MessageType)>
    to_error_response (It first, It last, std::error_code &error) const noexcept
  {
    return to_response<ProtocolTraits::to_error_response(MessageType)>(
      first, last, error
    );
  }


  /**
   * Create error response for this message into region [\a first, \a last).
   * On failure, throw \c std::system_error
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object in which case after call \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_error_response(MessageType)>
    to_error_response (It first, It last) const
  {
    return to_error_response(first, last,
      sal::throw_on_error("message_reader::to_error_response")
    );
  }


  /**
   * Create error response for this message into region \a data.
   * On failure, set \a error and returned object is in undefined state.
   *
   * Specified region \a data can overlap with memory area occupied by this
   * object in which case after call \a this becomes invalid.
   */
  template <typename Data>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_error_response(MessageType)>
    to_error_response (Data &data, std::error_code &error) const noexcept
  {
    using std::begin;
    using std::end;
    return to_response<ProtocolTraits::to_error_response(MessageType)>(
      begin(data), end(data), error
    );
  }


  /**
   * Create error response for this message into region \a data.
   * On failure, throw \c std::system_error
   *
   * Specified region \a data can overlap with memory area occupied by this
   * object in which case after call \a this becomes invalid.
   */
  template <typename Data>
  message_writer_t<ProtocolTraits, ProtocolTraits::to_error_response(MessageType)>
    to_error_response (Data &data) const
  {
    return to_error_response(data,
      sal::throw_on_error("message_reader::to_error_response")
    );
  }


private:

  static inline constexpr size_t header_size =
    turner::protocol_t<ProtocolTraits>::header_and_cookie_size();

  static inline constexpr uint16_t min_length =
    turner::protocol_t<ProtocolTraits>::min_payload_length();


  template <uint16_t ResponseMessageType, typename It>
  message_writer_t<ProtocolTraits, ResponseMessageType> to_response (
    It first, It last, std::error_code &error) const noexcept
  {
    static_assert(ProtocolTraits::is_request(MessageType),
      "expected request message type"
    );

    auto begin = sal::to_ptr(first), end = sal::to_end_ptr(first, last);
    if (begin + header_size <= end)
    {
      auto src = this->template as_ptr<uint8_t>();
      if (src != begin)
      {
        std::memmove(begin, src, header_size);
      }

      reinterpret_cast<uint16_t *>(begin)[0] =
        sal::native_to_network_byte_order(ResponseMessageType);

      reinterpret_cast<uint16_t *>(begin)[1] =
        sal::native_to_network_byte_order(min_length);

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
  bool good () const noexcept
  {
    return first_ + ProtocolTraits::header_size <= last_;
  }


  /**
   * \copydoc good()
   */
  explicit operator bool () const noexcept
  {
    return good();
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


  /**
   * Return finalized message as range [\a first, \a second). Calling this
   * method while object is not good() is undefined behaviour.
   */
  std::pair<const uint8_t *, const uint8_t *> finish () const noexcept
  {
    return { first_, first_ + ProtocolTraits::header_size + length() };
  }


  /**
   * \copydoc finish()
   * Also adds message integrity to the end of message. If remaining buffer is
   * too small for integrity, set \a error to errc::not_enough_room.
   */
  template <typename Digest>
  std::pair<const uint8_t *, const uint8_t *> finish (
    sal::crypto::hmac_t<Digest> &integrity_calculator,
    std::error_code &error
  ) noexcept;


  /**
   * \copydoc finish()
   * Also adds message integrity to the end of message.
   *
   * \throws std::system_error If remaining buffer is too small for integrity.
   */
  template <typename Digest>
  std::pair<const uint8_t *, const uint8_t *> finish (
    sal::crypto::hmac_t<Digest> &integrity_calculator)
  {
    return finish(integrity_calculator,
      sal::throw_on_error("message_writer::finish")
    );
  }


private:

  uint8_t * const first_, * const last_;

  message_writer_t (uint8_t *first, uint8_t *last) noexcept
    : first_(first)
    , last_(last)
  {}

  friend class turner::protocol_t<ProtocolTraits>;
  friend class message_reader_t<ProtocolTraits, ProtocolTraits::to_request(MessageType)>;
};


template <typename ProtocolTraits>
template <typename Digest>
bool any_message_t<ProtocolTraits>::has_valid_integrity (
  sal::crypto::hmac_t<Digest> &integrity_calculator,
  std::error_code &error) const noexcept
{
  auto payload = as_ptr<uint8_t>() + ProtocolTraits::header_size;
  if (auto integrity = __bits::find_attribute(payload, payload + length(),
      ProtocolTraits::message_integrity,
      ProtocolTraits::padding_size,
      error))
  {
    if (integrity->length() == integrity_calculator.digest_size)
    {
      // type and fixed length [first attribute, MESSAGE-INTEGRITY attribute)
      const uint16_t type_and_length[] =
      {
        as_ptr<uint16_t>()[0],
        sal::native_to_network_byte_order(
          static_cast<uint16_t>(
            (reinterpret_cast<const uint8_t *>(integrity) - payload)
            + 2 * sizeof(uint16_t)
            + integrity_calculator.digest_size
          )
        ),
      };
      integrity_calculator.update(type_and_length);

      // rest of message (up to MESSAGE-INTEGRITY)
      integrity_calculator.update(
        as_ptr<uint8_t>() + sizeof(type_and_length),
        reinterpret_cast<const uint8_t *>(integrity)
      );

      // padding if necessary
      if constexpr (ProtocolTraits::message_integrity_padding > 1)
      {
        static constexpr const
          std::array<uint8_t, ProtocolTraits::message_integrity_padding> pad{};
        auto size = reinterpret_cast<const uint8_t *>(integrity) - as_ptr<uint8_t>();
        if (size % pad.size() != 0)
        {
          auto pad_size = pad.size() - size % pad.size();
          integrity_calculator.update(pad.cbegin(), pad.cbegin() + pad_size);
        }
      }

      // check if valid
      auto expected = integrity_calculator.finish();
      if (std::equal(expected.begin(), expected.end(),
          integrity->begin(), integrity->end()))
      {
        error.clear();
        return true;
      }
      error = make_error_code(errc::unexpected_attribute_value);
    }
    else
    {
      error = make_error_code(errc::unexpected_attribute_length);
    }
  }
  return false;
}


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
    message_size += static_cast<uint16_t>(2 * sizeof(uint16_t) + attribute_size);
    if constexpr (ProtocolTraits::padding_size > 1)
    {
      constexpr const auto r = ProtocolTraits::padding_size - 1;
      message_size = (message_size + r) & ~r;
    }
    if (first_ + ProtocolTraits::header_size + message_size > last_)
    {
      error = make_error_code(errc::not_enough_room);
      return *this;
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


template <typename ProtocolTraits, uint16_t MessageType>
template <typename Digest>
std::pair<const uint8_t *, const uint8_t *>
  message_writer_t<ProtocolTraits, MessageType>::finish (
    sal::crypto::hmac_t<Digest> &integrity_calculator,
    std::error_code &error) noexcept
{
  uint16_t digest_size = integrity_calculator.digest_size;
  uint16_t original_length = length();
  uint16_t new_length = original_length + 2 * sizeof(uint16_t) + digest_size;

  if (first_ + new_length + ProtocolTraits::header_size <= last_)
  {
    // update message length
    reinterpret_cast<uint16_t *>(first_)[1] =
      sal::native_to_network_byte_order(new_length);

    // add MESSAGE-INTEGRITY type & length
    auto attribute = first_ + ProtocolTraits::header_size + original_length;
    reinterpret_cast<uint16_t *>(attribute)[0] =
      sal::native_to_network_byte_order(ProtocolTraits::message_integrity);
    reinterpret_cast<uint16_t *>(attribute)[1] =
      sal::native_to_network_byte_order(digest_size);

    // add MESSAGE-INTEGRITY value
    integrity_calculator.update(first_, attribute);
    if constexpr (ProtocolTraits::message_integrity_padding > 1)
    {
      static constexpr const
        std::array<uint8_t, ProtocolTraits::message_integrity_padding> pad{};
      auto size = attribute - first_;
      if (size % pad.size() != 0)
      {
        auto pad_size = pad.size() - size % pad.size();
        integrity_calculator.update(pad.cbegin(), pad.cbegin() + pad_size);
      }
    }
    attribute += 2 * sizeof(uint16_t);
    integrity_calculator.finish(attribute, attribute + digest_size);

    error.clear();
    return finish();
  }

  error = make_error_code(errc::not_enough_room);
  return {};
}


__turner_end
