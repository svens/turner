#pragma once

/**
 * \file turner/codec/turn.hpp
 * TURN protocol
 */

#include <turner/codec/method.hpp>
#include <turner/codec/stun.hpp>

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
};

} // namespace turner
