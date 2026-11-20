#pragma once

/**
 * \file turner/codec/protocol.hpp
 * Protocol and entity relationship
 */

#include <concepts>
#include <type_traits>

namespace turner
{

/// What received data starts with, as returned by \c peek() of protocol
enum class content
{
	need_more, ///< More bytes are needed to tell
	message, ///< Protocol message: try \c parse()
	data, ///< Frame of relayed data without message header
	other, ///< Something else; more bytes do not change this
};

/// Kind of transport data is received from
enum class transport
{
	datagram, ///< Each receive holds one whole unit: UDP
	stream, ///< Byte stream: TCP, TLS over TCP
};

/// True if \a Entity (method, message, attribute) is usable with \a Protocol: it is defined by \a Protocol or its base.
template <typename Entity, typename Protocol>
concept usable_with = std::derived_from<Protocol, typename std::remove_cvref_t<Entity>::protocol_type>;

} // namespace turner
