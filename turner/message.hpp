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
   * Protocol traits.
   */
  using traits_t = ProtocolTraits;

  /**
   * Protocol class describing raw network message format.
   */
  using protocol_t = turner::protocol_t<traits_t>;


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
    return traits_t::is_request(type());
  }


  /**
   * Return true if message type is success response.
   */
  bool is_success_response () const noexcept
  {
    return traits_t::is_success_response(type());
  }


  /**
   * Return true if message type is error response.
   */
  bool is_error_response () const noexcept
  {
    return traits_t::is_error_response(type());
  }


  /**
   * Return true if message type is indication.
   */
  bool is_indication () const noexcept
  {
    return traits_t::is_indication(type());
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
   * Return pointer to beginning of message.
   */
  const uint8_t *begin () const noexcept
  {
    return as_ptr<uint8_t>();
  }


  /**
   * Return pointer to one byte past end of the message.
   */
  const uint8_t *end () const noexcept
  {
    return as_ptr<uint8_t>() + traits_t::header_size + length();
  }


  /**
   * Return message transaction ID.
   */
  const transaction_id_t &transaction_id () const noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      as_ptr<uint8_t>() + traits_t::transaction_id_offset
    );
  }


  /**
   * Return concrete message instance from \a this generic message if
   * underlying raw data has expected \a MessageType. On different type,
   * return nullptr.
   */
  template <uint16_t MessageType>
  const message_reader_t<traits_t, MessageType> *try_as (
    message_type_t<traits_t, MessageType> message_type) const noexcept
  {
    return message_type(this);
  }


  /**
   * Return concrete message instance from \a this generic message if
   * underlying raw data has expected \a MessageType.
   *
   * \throws std::system_error if \a this message type is not \a MessageType.
   */
  template <uint16_t MessageType>
  const message_reader_t<traits_t, MessageType> &as (
    message_type_t<traits_t, MessageType> message_type) const
  {
    if (auto msg = message_type(this))
    {
      return *msg;
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
  template <typename T>
  const T *as_ptr () const noexcept
  {
    return reinterpret_cast<const T *>(this);
  }

  const uint8_t *payload () const noexcept
  {
    return as_ptr<uint8_t>() + protocol_t::header_and_cookie_size();
  }
  /// \endcond


private:

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
   * Protocol traits.
   */
  using traits_t = ProtocolTraits;

  /**
   * Protocol class describing raw network message format.
   */
  using protocol_t = turner::protocol_t<traits_t>;

  /**
   * Message type.
   */
  static inline constexpr const message_type_t<traits_t, MessageType> type{};


  /**
   * Decode and return \a AttributeType value from message. On failure set \a
   * error to code describing failure and return default value.
   */
  template <
    uint16_t AttributeType,
    template <typename> typename AttributeProcessor
  >
  typename AttributeProcessor<traits_t>::value_t read (
    attribute_type_t<traits_t, AttributeType, AttributeProcessor>,
    std::error_code &error) const noexcept
  {
    if (auto attribute = __bits::find_attribute(this->payload(), this->end(),
        AttributeType,
        traits_t::padding_size,
        error))
    {
      return AttributeProcessor<traits_t>::read(*this, *attribute, error);
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
  typename AttributeProcessor<traits_t>::value_t read (
    attribute_type_t<traits_t, AttributeType, AttributeProcessor> attribute) const
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
  message_writer_t<traits_t, MessageType | 0b0000'0001'0000'0000>
    to_success_response (It first, It last, std::error_code &error) const noexcept
  {
    return to_response<MessageType | 0b0000'0001'0000'0000>(first, last, error);
  }


  /**
   * Create success response for this message into region [\a first, \a last).
   * On failure, throw \c std::system_error
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object in which case after call \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<traits_t, MessageType | 0b0000'0001'0000'0000>
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
  message_writer_t<traits_t, MessageType | 0b0000'0001'0000'0000>
    to_success_response (Data &data, std::error_code &error) const noexcept
  {
    using std::begin;
    using std::end;
    return to_response<MessageType | 0b0000'0001'0000'0000>(
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
  message_writer_t<traits_t, MessageType | 0b0000'0001'0000'0000>
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
  message_writer_t<traits_t, MessageType | 0b000'0001'0001'0000>
    to_error_response (It first, It last, std::error_code &error) const noexcept
  {
    return to_response<MessageType | 0b000'0001'0001'0000>(first, last, error);
  }


  /**
   * Create error response for this message into region [\a first, \a last).
   * On failure, throw \c std::system_error
   *
   * Specified region [\a first, \a last) can overlap with memory area
   * occupied by this object in which case after call \a this becomes invalid.
   */
  template <typename It>
  message_writer_t<traits_t, MessageType | 0b000'0001'0001'0000>
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
  message_writer_t<traits_t, MessageType | 0b000'0001'0001'0000>
    to_error_response (Data &data, std::error_code &error) const noexcept
  {
    using std::begin;
    using std::end;
    return to_response<MessageType | 0b000'0001'0001'0000>(
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
  message_writer_t<traits_t, MessageType | 0b000'0001'0001'0000>
    to_error_response (Data &data) const
  {
    return to_error_response(data,
      sal::throw_on_error("message_reader::to_error_response")
    );
  }


private:

  template <uint16_t ResponseMessageType, typename It>
  message_writer_t<ProtocolTraits, ResponseMessageType> to_response (
    It first, It last, std::error_code &error) const noexcept
  {
    static_assert(traits_t::is_request(MessageType),
      "expected request message type"
    );

    auto begin = sal::to_ptr(first), end = sal::to_end_ptr(first, last);
    if (begin + protocol_t::header_and_cookie_size() <= end)
    {
      auto src = this->template as_ptr<uint8_t>();
      if (src != begin)
      {
        std::memmove(begin, src, protocol_t::header_and_cookie_size());
      }

      reinterpret_cast<uint16_t *>(begin)[0] =
        sal::native_to_network_byte_order(ResponseMessageType);

      reinterpret_cast<uint16_t *>(begin)[1] =
        sal::native_to_network_byte_order(protocol_t::min_payload_length());

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
   * Protocol traits.
   */
  using traits_t = ProtocolTraits;


  /**
   * Protocol class describing raw network message format.
   */
  using protocol_t = turner::protocol_t<traits_t>;


  /**
   * Per message unique transaction ID.
   */
  using transaction_id_t = typename protocol_t::transaction_id_t;


  /**
   * Message type.
   */
  static inline constexpr const message_type_t<traits_t, MessageType> type{};


  /**
   * Return true if this object is in valid state. Calling any other method is
   * undefined behavious if object is not in valid state.
   */
  bool good () const noexcept
  {
    return first_ + traits_t::header_size <= last_;
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
   * Return message transaction ID.
   */
  const transaction_id_t &transaction_id () const noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      sal::to_ptr(first_) + traits_t::transaction_id_offset
    );
  }


  /**
   * Return available room for building message (in bytes).
   */
  uint16_t available () const noexcept
  {
    return static_cast<uint16_t>(
      last_ - first_ - traits_t::header_size - length()
    );
  }


  /**
   * Encode \a value and append to message as \a AttributeType. If there is
   * no enough room in internal buffer, \a set error to errc::not_enough_room
   * and do nothing.
   */
  template <uint16_t AttributeType, template <typename> typename AttributeProcessor>
  message_writer_t &write (
    attribute_type_t<traits_t, AttributeType, AttributeProcessor>,
    const typename AttributeProcessor<traits_t>::value_t &value,
    std::error_code &error
  ) noexcept;


  /**
   * Encode \a value and append to message as \a AttributeType. If there is
   * no enough room in internal buffer, throw std::system_error.
   */
  template <uint16_t AttributeType, template <typename> typename AttributeProcessor>
  message_writer_t &write (
    attribute_type_t<traits_t, AttributeType, AttributeProcessor> attribute,
    const typename AttributeProcessor<traits_t>::value_t &value)
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
    return { first_, first_ + traits_t::header_size + length() };
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

  friend class message_type_t<traits_t, MessageType>;
  friend class message_reader_t<traits_t, MessageType & ~0b0000'0001'0001'0000>;
};


template <typename ProtocolTraits>
template <typename Digest>
bool any_message_t<ProtocolTraits>::has_valid_integrity (
  sal::crypto::hmac_t<Digest> &integrity_calculator,
  std::error_code &error) const noexcept
{
  if (auto integrity = __bits::find_attribute(payload(), end(),
      traits_t::message_integrity,
      traits_t::padding_size,
      error))
  {
    if (integrity->length() == integrity_calculator.digest_size)
    {
      std::array<uint8_t, 4096> buf;
      auto n = reinterpret_cast<const uint8_t *>(integrity) - as_ptr<uint8_t>();
      std::uninitialized_copy_n(as_ptr<uint8_t>(), n, buf.begin());

      reinterpret_cast<uint16_t *>(buf.data())[1] =
        sal::native_to_network_byte_order(
          static_cast<uint16_t>(n
            - traits_t::header_size
            + 2 * sizeof(uint16_t)
            + integrity_calculator.digest_size
          )
        );

      if constexpr (traits_t::message_integrity_padding > 1)
      {
        static constexpr const auto r = traits_t::message_integrity_padding - 1;
        if (n % traits_t::message_integrity_padding != 0)
        {
          auto new_n = (n + r) & ~r;
          std::fill(buf.begin() + n, buf.begin() + new_n, '\0');
          n = new_n;
        }
      }

      auto expected = integrity_calculator
        .update(buf.begin(), buf.begin() + n)
        .finish();

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


/**
 * Encode \a value and append to message as \a AttributeType. If there is
 * no enough room in internal buffer, \a set error to errc::not_enough_room
 * and do nothing.
 */
template <typename ProtocolTraits, uint16_t MessageType>
template <uint16_t AttributeType, template <typename> typename AttributeProcessor>
auto message_writer_t<ProtocolTraits, MessageType>::write (
  attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor>,
  const typename AttributeProcessor<ProtocolTraits>::value_t &value,
  std::error_code &error) noexcept -> message_writer_t &
{
  auto &message = *reinterpret_cast<any_message_t<traits_t> *>(first_);
  auto message_size = message.length();

  auto attribute = first_ + traits_t::header_size + message_size;
  auto attribute_size = AttributeProcessor<traits_t>::write(message,
    attribute + 2 * sizeof(uint16_t),
    last_,
    value,
    error
  );

  if (!error)
  {
    message_size += static_cast<uint16_t>(2 * sizeof(uint16_t) + attribute_size);
    if constexpr (traits_t::padding_size > 1)
    {
      constexpr const auto r = traits_t::padding_size - 1;
      message_size = (message_size + r) & ~r;
      if (first_ + traits_t::header_size + message_size > last_)
      {
        error = make_error_code(errc::not_enough_room);
        return *this;
      }
    }

    reinterpret_cast<uint16_t *>(first_)[1] =
      sal::native_to_network_byte_order(message_size);
    reinterpret_cast<uint16_t *>(attribute)[0] =
      sal::native_to_network_byte_order(AttributeType);
    reinterpret_cast<uint16_t *>(attribute)[1] =
      sal::native_to_network_byte_order(static_cast<uint16_t>(attribute_size));
  }

  return *this;
}


/**
 * \copydoc finish()
 * Also adds message integrity to the end of message. If remaining buffer is
 * too small for integrity, set \a error to errc::not_enough_room.
 */
template <typename ProtocolTraits, uint16_t MessageType>
template <typename Digest>
auto message_writer_t<ProtocolTraits, MessageType>::finish (
  sal::crypto::hmac_t<Digest> &integrity_calculator,
  std::error_code &error) noexcept -> std::pair<const uint8_t *, const uint8_t *>
{
  uint16_t digest_size = integrity_calculator.digest_size;
  uint16_t original_length = length();
  uint16_t new_length = original_length + 2 * sizeof(uint16_t) + digest_size;

  if (first_ + new_length + traits_t::header_size <= last_)
  {
    reinterpret_cast<uint16_t *>(first_)[1] =
      sal::native_to_network_byte_order(new_length);

    // integrity type & length
    auto attribute = first_ + traits_t::header_size + original_length;
    reinterpret_cast<uint16_t *>(attribute)[0] =
      sal::native_to_network_byte_order(traits_t::message_integrity);
    reinterpret_cast<uint16_t *>(attribute)[1] =
      sal::native_to_network_byte_order(digest_size);

    // integrity value
    integrity_calculator.update(first_, attribute);
    if constexpr (traits_t::message_integrity_padding > 1)
    {
      static constexpr const
        std::array<uint8_t, traits_t::message_integrity_padding> pad{};
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
