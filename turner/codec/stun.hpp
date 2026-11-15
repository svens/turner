#pragma once

/**
 * \file turner/codec/stun.hpp
 * STUN protocol
 */

#include <turner/codec/method.hpp>
#include <array>
#include <cstddef>

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
	static constexpr auto binding = method<stun, 0x001, message_class::request_response | message_class::indication>;
};

} // namespace turner
