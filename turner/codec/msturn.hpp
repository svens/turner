#pragma once

/**
 * \file turner/codec/msturn.hpp
 * MS-TURN protocol
 */

#include <turner/codec/method.hpp>
#include <turner/codec/reader.hpp>
#include <pal/result.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace turner
{

struct msturn;

namespace __method
{

template <>
inline constexpr uint16_t wire_type<msturn, 0x005, message_class::indication> = 0x0115;

} // namespace __method

/**
 * MS-TURN protocol
 *
 * \see https://docs.microsoft.com/en-us/openspecs/office_protocols/ms-turn
 */
struct msturn
{
	/// Transaction ID
	using transaction_id = std::array<std::byte, 16>;

	/// Offset of transaction ID from beginning of message
	static constexpr size_t transaction_id_offset = 4;

	/// Message header size. Header length field does not count it.
	static constexpr size_t header_size = 20;

	/// Offset of \ref cookie from beginning of message
	static constexpr size_t cookie_offset = header_size;

	/// Magic Cookie: attribute following message header, with its type and length fields
	static constexpr std::array cookie =
	{
		std::byte{0x00}, std::byte{0x0f}, std::byte{0x00}, std::byte{0x04},
		std::byte{0x72}, std::byte{0xc6}, std::byte{0x4b}, std::byte{0xc6},
	};

	/// Attribute padding boundary
	static constexpr size_t attribute_alignment = 1;

	/// Allocate
	static constexpr auto allocate = request<msturn, 0x003>;

	/// Send Request: request without responses
	static constexpr auto send_request = method<msturn, 0x004, message_class::request>;

	/// Data Indication
	static constexpr auto data_indication = indication<msturn, 0x005>;

	/// Set Active Destination
	static constexpr auto set_active_destination = request<msturn, 0x006>;

	/// Validate message in first \a received bytes of \a buffer. \a buffer is whole I/O buffer: response to the
	/// message is built in it. \a received must not exceed \a buffer size.
	///
	/// Checks message structure only (header, Magic Cookie, attribute layout), not whether message type or
	/// attributes are known. Fails with errc::incomplete if \a received bytes do not hold whole message.
	/// Bytes following the message are ignored: use parsed_message::size() to find them.
	[[nodiscard]] static pal::result<parsed_message<msturn>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;
};

} // namespace turner
