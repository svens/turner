#pragma once

/**
 * \file turner/codec/reader.hpp
 * Parsed message and typed message reader
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <pal/packed_struct.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace turner
{

template <typename Protocol>
class parsed_message;

namespace __reader
{

inline const __wire::type_and_length &type_and_length (std::span<std::byte> buffer) noexcept
{
	return pal::packed_as<__wire::type_and_length>(buffer.first<sizeof(__wire::type_and_length)>());
}

template <typename Protocol>
const typename Protocol::transaction_id &transaction_id (std::span<std::byte> buffer) noexcept
{
	using id_type = typename Protocol::transaction_id;
	return pal::packed_as<id_type>(buffer.subspan<Protocol::transaction_id_offset, sizeof(id_type)>());
}

} // namespace __reader

/// Reader of validated message of type \a Message (message_type). Created by parsed_message::as().
template <typename Message>
class basic_message_reader
{
public:

	/// Protocol that defines the message
	using protocol_type = typename Message::protocol_type;

	/// Return message transaction ID. Reference is valid while message buffer content is.
	[[nodiscard]] const typename protocol_type::transaction_id &transaction_id () const noexcept
	{
		return __reader::transaction_id<protocol_type>(buffer_);
	}

private:

	std::span<std::byte> buffer_;

	explicit basic_message_reader (std::span<std::byte> buffer) noexcept
		: buffer_{buffer}
	{
	}

	template <typename>
	friend class parsed_message;
};

/// Reader of message that \a Entity denotes: message, or method with single initiating class
template <auto Entity>
using message_reader = basic_message_reader<message_of<decltype(Entity)>>;

/// Validated \a Protocol message of yet unknown type. Created by \c Protocol::parse().
template <typename Protocol>
class parsed_message
{
public:

	/// Protocol the message was parsed as
	using protocol_type = Protocol;

	/// Return message size in bytes, header included
	[[nodiscard]] size_t size () const noexcept
	{
		return Protocol::header_size + __reader::type_and_length(buffer_).length;
	}

	/// Return message type as it appears in message header (host byte order)
	[[nodiscard]] uint16_t type () const noexcept
	{
		return __reader::type_and_length(buffer_).type;
	}

	/// Return message transaction ID. Reference is valid while message buffer content is.
	[[nodiscard]] const typename Protocol::transaction_id &transaction_id () const noexcept
	{
		return __reader::transaction_id<Protocol>(buffer_);
	}

	/// Return reader of message \a M if this is one, otherwise \c std::nullopt
	template <message M>
		requires usable_with<M, Protocol>
	[[nodiscard]] std::optional<basic_message_reader<message_of<M>>> as (M) const noexcept
	{
		if (type() == message_of<M>::type)
		{
			return basic_message_reader<message_of<M>>{buffer_};
		}
		return std::nullopt;
	}

private:

	std::span<std::byte> buffer_;

	explicit parsed_message (std::span<std::byte> buffer) noexcept
		: buffer_{buffer}
	{
	}

	friend Protocol;
};

} // namespace turner
