#pragma once

/**
 * \file turner/codec/sniff.hpp
 * Protocol of new session
 */

#include <turner/codec/protocol.hpp>
#include <cstddef>
#include <span>

namespace turner
{

/// Protocol that first bytes of session belong to, as returned by sniff()
enum class session_protocol
{
	need_more, ///< More bytes are needed to tell
	stun, ///< STUN or TURN: for TURN server, continue with turn::peek() and turn::parse()
	msturn, ///< MS-TURN: continue with msturn::peek() and msturn::parse(), on stream with msturn::parse_frame()
	other, ///< None of these; more bytes do not change this
};

/// Return protocol of session that starts with \a bytes received on \a transport.
///
/// On transport::datagram, \a bytes is whole datagram, so session_protocol::need_more is not returned.
/// Multiplexed TURN packet is session_protocol::other: use msturn::peek() for these on shared relay port.
///
/// On transport::stream, \a bytes is what is received so far, with TLS already removed. First byte of STUN or TURN
/// session is 0 or 1, and MS-TURN session starts with control frame, whose first byte is 2. Anything else is
/// session_protocol::other. Pseudo-TLS is not recognized here: use msturn::parse_pseudo_tls() first on raw connection.
[[nodiscard]] session_protocol sniff (std::span<const std::byte> bytes, transport transport) noexcept;

} // namespace turner
