#pragma once

/**
 * \file turner/codec/turn.hpp
 * TURN protocol
 */

#include <turner/codec/attribute.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/value.hpp>
#include <pal/result.hpp>
#include <cstddef>
#include <span>

namespace turner
{

/**
 * TURN protocol
 *
 * \see https://datatracker.ietf.org/doc/html/rfc8656
 */
struct turn: stun
{
	/// Allocate
	static constexpr auto allocate = request<turn, 0x003>;

	/// Refresh
	static constexpr auto refresh = request<turn, 0x004>;

	/// Send indication
	static constexpr auto send_indication = indication<turn, 0x006>;

	/// Data indication
	static constexpr auto data_indication = indication<turn, 0x007>;

	/// CreatePermission
	static constexpr auto create_permission = request<turn, 0x008>;

	/// ChannelBind
	static constexpr auto channel_bind = request<turn, 0x009>;

	/// LIFETIME
	static constexpr auto lifetime = attribute<turn, seconds_value, 0x000d>;

	/// DATA
	static constexpr auto data = attribute<turn, bytes_value<>, 0x0013>;

	/// REQUESTED-ADDRESS-FAMILY
	static constexpr auto requested_address_family = attribute<turn, address_family_value, 0x0017>;

	/// REQUESTED-TRANSPORT
	static constexpr auto requested_transport = attribute<turn, transport_protocol_value, 0x0019>;

	/// DONT-FRAGMENT
	static constexpr auto dont_fragment = attribute<turn, flag_value, 0x001a>;

	/// RESERVATION-TOKEN
	static constexpr auto reservation_token = attribute<turn, bytes_value<8>, 0x0022>;

	/// ADDITIONAL-ADDRESS-FAMILY
	static constexpr auto additional_address_family = attribute<turn, address_family_value, 0x8000>;

	/// \copydoc stun::parse
	[[nodiscard]] static pal::result<parsed_message<turn>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;
};

} // namespace turner
