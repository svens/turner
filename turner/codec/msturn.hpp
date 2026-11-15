#pragma once

/**
 * \file turner/codec/msturn.hpp
 * MS-TURN protocol
 */

#include <turner/codec/method.hpp>
#include <array>
#include <cstddef>
#include <cstdint>

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
};

} // namespace turner
