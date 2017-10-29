#pragma once

/**
 * \file turner/message.hpp
 * Protocol message
 */

#include <turner/config.hpp>
#include <turner/fwd.hpp>
#include <turner/error.hpp>
#include <sal/byte_order.hpp>


__turner_begin


/**
 * Generic protocol message, not tied to any concrete message type.
 *
 * It can be used to parse STUN, TURN or MSTURN protocol messages only as it's
 * header layout depends on corresponding protocol header layout.
 *
 * This type is not meant to be instantiated directly but from
 * protocol_t<ProtocolTraits>::from_wire() that does message validation.
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
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(this)[0]
    );
  }


  /**
   * Return message size as claimed by message length field. It does not
   * include message header length.
   */
  uint16_t length () const noexcept
  {
    return sal::network_to_native_byte_order(
      reinterpret_cast<const uint16_t *>(this)[1]
    );
  }


  /**
   * Return message cookie.
   */
  const cookie_t &cookie () const noexcept
  {
    return *reinterpret_cast<const cookie_t *>(
      __bits::to_ptr(this) + ProtocolTraits::cookie_offset
    );
  }


  /**
   * Return message transaction ID.
   */
  const transaction_id_t &transaction_id () const noexcept
  {
    return *reinterpret_cast<const transaction_id_t *>(
      __bits::to_ptr(this) + ProtocolTraits::transaction_id_offset
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
      ? reinterpret_cast<const message_reader_t<ProtocolTraits, MessageType> *>(this)
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
      return *reinterpret_cast<const message_reader_t<ProtocolTraits, MessageType> *>(this);
    }
    unexpected_message_type("any_message::as");
  }


private:

  template <size_t N>
  static void unexpected_message_type [[noreturn]] (const char (&msg)[N])
  {
    sal::throw_system_error(
      make_error_code(errc::unexpected_message_type), msg
    );
  }

  any_message_t () = delete;
  any_message_t (const any_message_t &) = delete;
  any_message_t &operator= (const any_message_t &) = delete;
  any_message_t (any_message_t &&) = delete;
  any_message_t &operator= (any_message_t &&) = delete;
};


/**
 * Concrete protocol's message.
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
  template <uint16_t AttributeType, typename AttributeProcessor>
  typename AttributeProcessor::value_t read (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor>,
    std::error_code &error
  ) const noexcept;


  /**
   * Decode and return \a AttributeType value from message. On failure throw
   * \a std::system_error.
   */
  template <uint16_t AttributeType, typename AttributeProcessor>
  typename AttributeProcessor::value_t read (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor> attribute) const
  {
    return read(attribute, sal::throw_on_error("message_reader::read"));
  }


private:

  const any_attribute_t *begin () const noexcept
  {
    return reinterpret_cast<const any_attribute_t *>(
      __bits::to_ptr(this) + ProtocolTraits::header_size
    );
  }

  const any_attribute_t *end () const noexcept
  {
    return reinterpret_cast<const any_attribute_t *>(
      __bits::to_ptr(this) + ProtocolTraits::header_size + this->length()
    );
  }
};


template <typename ProtocolTraits, uint16_t MessageType>
template <uint16_t AttributeType, typename AttributeProcessor>
typename AttributeProcessor::value_t
  message_reader_t<ProtocolTraits, MessageType>::read (
    attribute_type_t<ProtocolTraits, AttributeType, AttributeProcessor>,
    std::error_code &error) const noexcept
{
  for (auto it = begin(), e = end();  it < e;  it = it->next())
  {
    if (it->type() == AttributeType)
    {
      if (it->data() + it->length() <= __bits::to_ptr(e))
      {
        return AttributeProcessor::read(*this, *it, error);
      }
      else
      {
        error = make_error_code(errc::unexpected_attribute_length);
        return {};
      }
    }
  }
  error = make_error_code(errc::attribute_not_found);
  return {};
}


__turner_end
