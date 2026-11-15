#pragma once

/**
 * \file turner/codec/stun.hpp
 * STUN protocol
 */

#include <turner/codec/attribute.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/value.hpp>
#include <pal/result.hpp>
#include <array>
#include <cstddef>
#include <span>

namespace turner
{

/**
 * STUN protocol
 *
 * \see https://datatracker.ietf.org/doc/html/rfc8489
 */
struct stun
{
	/// Transaction ID
	using transaction_id = std::array<std::byte, 12>;

	/// Offset of transaction ID from beginning of message
	static constexpr size_t transaction_id_offset = 8;

	/// Message header size. Header length field does not count it.
	static constexpr size_t header_size = 20;

	/// Offset of \ref cookie from beginning of message
	static constexpr size_t cookie_offset = 4;

	/// Magic Cookie
	static constexpr std::array cookie =
	{
		std::byte{0x21}, std::byte{0x12}, std::byte{0xa4}, std::byte{0x42},
	};

	/// Attribute padding boundary
	static constexpr size_t attribute_alignment = 4;

	/// Binding
	static constexpr auto binding = method<
		stun,
		0x001,
		message_class::request_response | message_class::indication
	>;

	/// USERNAME
	static constexpr auto username = attribute<stun, string_value<508>, 0x0006>;

	/// REALM
	static constexpr auto realm = attribute<stun, string_value<763>, 0x0014>;

	/// NONCE
	static constexpr auto nonce = attribute<stun, string_value<763>, 0x0015>;

	/// SOFTWARE
	static constexpr auto software = attribute<stun, string_value<763>, 0x8022>;

	/// Validate message in first \a received bytes of \a buffer. \a buffer is whole I/O buffer: response to the
	/// message is built in it. \a received must not exceed \a buffer size.
	///
	/// Checks message structure only (header, attribute layout), not whether message type or attributes are
	/// known. Fails with errc::incomplete if \a received bytes do not hold whole message. Bytes following
	/// the message are ignored: use parsed_message::size() to find them.
	[[nodiscard]] static pal::result<parsed_message<stun>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;
};

} // namespace turner
