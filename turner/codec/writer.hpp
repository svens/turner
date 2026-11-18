#pragma once

/**
 * \file turner/codec/writer.hpp
 * Typed message writer
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

namespace turner
{

namespace __writer
{

template <typename Protocol>
void write_header (
	std::span<std::byte> message,
	uint16_t type,
	const typename Protocol::transaction_id &transaction_id) noexcept
{
	using id_type = typename Protocol::transaction_id;
	__wire::type_and_length::write(message, type, 0);
	std::ranges::copy(
		Protocol::cookie,
		message.subspan<Protocol::cookie_offset, Protocol::cookie.size()>().begin()
	);
	std::ranges::copy(transaction_id, message.subspan<Protocol::transaction_id_offset, sizeof(id_type)>().begin());
}

} // namespace __writer

/// Writer of message of type \a Message (message_type) into caller's buffer. First failure is kept: later
/// writes do nothing and finish() reports it. Response writers come from basic_message_reader::to_success()
/// and basic_message_reader::to_error().
template <typename Message>
class basic_message_writer
{
public:

	/// Protocol that defines the message
	using protocol_type = typename Message::protocol_type;

	/// Start writing request or indication with \a transaction_id into \a buffer. If \a buffer is too small
	/// for message header (for MS-TURN, header and Magic Cookie), finish() fails with errc::buffer_too_small.
	basic_message_writer (
		std::span<std::byte> buffer,
		const typename protocol_type::transaction_id &transaction_id) noexcept
		requires(__method::has(message_class::request | message_class::indication, __method::class_of<Message>))
	{
		if (buffer.size() < size_)
		{
			error_ = errc::buffer_too_small;
			return;
		}
		buffer_ = buffer.first((std::min)(buffer.size(), max_message_size));
		__writer::write_header<protocol_type>(buffer_, Message::type, transaction_id);
	}

	/// Append attribute with \a value, which must not refer to bytes of the buffer. Fails with
	/// errc::unexpected_attribute_length if size of \a value is outside bounds of attribute value type, and
	/// with errc::buffer_too_small if attribute does not fit into buffer or into what message length field
	/// can express. Not available for attribute without value (flag).
	template <typename Self, typename P, attribute_value V, uint16_t Type>
		requires usable_with<attribute_type<P, V, Type>, protocol_type>
			&& (!std::is_empty_v<typename V::native_type>)
			&& (!std::is_const_v<std::remove_reference_t<Self>>)
	Self &&write (this Self &&self, attribute_type<P, V, Type>, const typename V::native_type &value) noexcept
	{
		self.template append<V>(Type, value);
		return std::forward<Self>(self);
	}

	/// Append attribute without value (flag). Fails with errc::buffer_too_small if attribute does not fit
	/// into buffer or into what message length field can express.
	template <typename Self, typename P, attribute_value V, uint16_t Type>
		requires usable_with<attribute_type<P, V, Type>, protocol_type>
			&& std::is_empty_v<typename V::native_type>
			&& (!std::is_const_v<std::remove_reference_t<Self>>)
	Self &&write (this Self &&self, attribute_type<P, V, Type>) noexcept
	{
		self.template append<V>(Type, typename V::native_type{});
		return std::forward<Self>(self);
	}

	/// Return written message, or first failure
	[[nodiscard]] pal::result<std::span<const std::byte>> finish () && noexcept
	{
		return check_room(0).transform([this]() noexcept
		{
			return complete(size_);
		});
	}

	/// Append FINGERPRINT as last attribute and return written message, or first failure. Fails with
	/// errc::buffer_too_small if FINGERPRINT does not fit into buffer or into what message length field can
	/// express. Available only for protocols that define FINGERPRINT (stun::fingerprint).
	template <std::same_as<typename protocol_type::fingerprint_type> Fingerprint>
	[[nodiscard]] pal::result<std::span<const std::byte>> finish (Fingerprint) && noexcept
	{
		return check_room(Fingerprint::size).transform([this]() noexcept
		{
			return append_fingerprint<Fingerprint>();
		});
	}

	/// Append integrity attribute matching \a key as last attribute and return written message, or first
	/// failure. Fails with errc::buffer_too_small if attribute does not fit into buffer or into what message
	/// length field can express.
	template <typename P>
		requires std::derived_from<protocol_type, P>
	[[nodiscard]] pal::result<std::span<const std::byte>> finish (integrity_key<P> &key) && noexcept
	{
		const auto attribute_size = integrity_attribute_size(key.hmac_);
		return check_room(attribute_size).transform([&]() noexcept
		{
			return append_integrity(key.hmac_, attribute_size);
		});
	}

	/// As finish(integrity_key<P> &), followed by FINGERPRINT as last attribute. Available only for protocols
	/// that define FINGERPRINT (stun::fingerprint).
	template <typename P, std::same_as<typename protocol_type::fingerprint_type> Fingerprint>
		requires std::derived_from<protocol_type, P>
	[[nodiscard]] pal::result<std::span<const std::byte>> finish (integrity_key<P> &key, Fingerprint) && noexcept
	{
		const auto attribute_size = integrity_attribute_size(key.hmac_);
		return check_room(attribute_size + Fingerprint::size).transform([&]() noexcept
		{
			append_integrity(key.hmac_, attribute_size);
			return append_fingerprint<Fingerprint>();
		});
	}

private:

	static constexpr size_t max_message_size = protocol_type::header_size + 0xffff;

	std::span<std::byte> buffer_;
	size_t size_ = __wire::attributes_offset<protocol_type>;
	errc error_{};

	explicit basic_message_writer (std::span<std::byte> buffer) noexcept
		: buffer_{buffer.first((std::min)(buffer.size(), max_message_size))}
	{
		__wire::type_and_length::write(buffer_, Message::type, 0);
	}

	[[nodiscard]] pal::result<void> check_room (size_t size) const noexcept
	{
		if (error_ != errc{})
		{
			return make_unexpected(error_);
		}
		if (size_ + size > buffer_.size())
		{
			return make_unexpected(errc::buffer_too_small);
		}
		return {};
	}

	std::span<const std::byte> complete (size_t size) noexcept
	{
		__wire::type_and_length::write(buffer_, Message::type, size - protocol_type::header_size);
		return buffer_.first(size);
	}

	[[nodiscard]] static size_t integrity_attribute_size (const __integrity::hmac &key) noexcept
	{
		return __wire::attribute_header_size + key.digest_size();
	}

	std::span<const std::byte> append_integrity (__integrity::hmac &key, size_t attribute_size) noexcept
	{
		const auto message = complete(size_ + attribute_size);
		protocol_type::integrity_type::write(key, buffer_.first(message.size()), size_);
		size_ = message.size();
		return message;
	}

	template <typename Fingerprint>
	std::span<const std::byte> append_fingerprint () noexcept
	{
		const auto message = complete(size_ + Fingerprint::size);
		Fingerprint::write(buffer_.subspan(size_).template first<Fingerprint::size>(), buffer_.first(size_));
		size_ = message.size();
		return message;
	}

	void fail (errc error) noexcept
	{
		if (error_ == errc{})
		{
			error_ = error;
		}
		buffer_ = {};
	}

	template <attribute_value V>
	void append (uint16_t type, const typename V::native_type &value) noexcept
	{
		const auto length = V::size(value);
		if (length < V::min_size || length > V::max_size)
		{
			fail(errc::unexpected_attribute_length);
			return;
		}

		constexpr auto attribute_header_size = __wire::attribute_header_size;
		const auto padded_length = __wire::align_up(length, protocol_type::attribute_alignment);
		if (size_ + attribute_header_size + padded_length > buffer_.size())
		{
			fail(errc::buffer_too_small);
			return;
		}

		const auto attribute = buffer_.subspan(size_, attribute_header_size + padded_length);
		__wire::type_and_length::write(attribute, type, length);
		V::write(buffer_.first(size_), attribute.subspan(attribute_header_size, length), value);
		std::ranges::fill(attribute.subspan(attribute_header_size + length), std::byte{});
		size_ += attribute_header_size + padded_length;
	}

	template <typename>
	friend class basic_message_reader;
};

/// Writer of message that \a Entity denotes: message, or method with single initiating class
template <auto Entity>
using message_writer = basic_message_writer<message_of<decltype(Entity)>>;

} // namespace turner
