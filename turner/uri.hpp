#pragma once

/**
 * \file turner/uri.hpp
 * STUN and TURN URI (RFC 7064, RFC 7065) parser and generator
 */

#include <turner/codec/protocol.hpp>
#include <pal/net/ip/port_type.hpp>
#include <pal/result.hpp>
#include <pal/uri.hpp>
#include <charconv>
#include <cstddef>
#include <string_view>

namespace turner
{

/// URI scheme
enum class uri_scheme
{
	stun, ///< \c stun (RFC 7064)
	stuns, ///< \c stuns (RFC 7064): over TLS or DTLS
	turn, ///< \c turn (RFC 7065)
	turns, ///< \c turns (RFC 7065): over TLS or DTLS
};

/// Return port used when URI with \a scheme has none: pal::net::ip::port_type::stun for \c stun and \c turn,
/// pal::net::ip::port_type::stuns for \c stuns and \c turns.
constexpr pal::net::ip::port_type default_port (uri_scheme scheme) noexcept
{
	return scheme == uri_scheme::stuns || scheme == uri_scheme::turns
		? pal::net::ip::port_type::stuns
		: pal::net::ip::port_type::stun
	;
}

/// Return transport used when URI with \a scheme has none: transport::datagram for \c stun and \c turn,
/// transport::stream for \c stuns and \c turns.
constexpr transport default_transport (uri_scheme scheme) noexcept
{
	return scheme == uri_scheme::stuns || scheme == uri_scheme::turns ? transport::stream : transport::datagram;
}

/**
 * STUN or TURN URI: <tt>scheme ":" host [":" port] ["?transport=" ("udp" / "tcp")]</tt>.
 *
 * Query \c transport=udp is transport::datagram (DTLS with secure scheme) and \c transport=tcp is transport::stream
 * (TLS with secure scheme). RFC 7064 has no query; accepting and writing it with \c stun and \c stuns is an extension.
 * RFC 7065 \c transport-ext values, IPvFuture, IPv6 zone identifiers and empty host are rejected. Empty port means
 * none.
 *
 * Reg-name host is a view into parsed text, still percent-encoded, and is compared as is: \c stun:H and \c stun:h
 * differ, so do \c stun:%41 and \c stun:A.
 */
struct uri_view
{
	/// Scheme
	uri_scheme scheme{};

	/// Host
	pal::uri_host host{};

	/// Port; default_port() of \a scheme if URI has none
	pal::net::ip::port_type port{};

	/// Transport; default_transport() of \a scheme if URI has none
	turner::transport transport{};

	/// Return URI parsed from all of \a text. Host refers to \a text.
	/// On failure returns std::errc::invalid_argument.
	[[nodiscard]] static pal::result<uri_view> make (std::string_view text) noexcept;

	/**
	 * Parse URI from all of [\a first, \a last). Scheme, query key and transport name are case-insensitive. Host
	 * refers to parsed text.
	 *
	 * On success returns std::from_chars_result with ptr \a last and ec zero. On failure ec is
	 * std::errc::invalid_argument; ptr and \a this are unspecified.
	 */
	[[nodiscard]] std::from_chars_result from_chars (const char *first, const char *last) noexcept;

	/**
	 * Write URI into [\a first, \a last), omitting port and transport equal to scheme defaults. Scheme, query key
	 * and transport name are written in lowercase, host as pal::uri_host::to_chars() writes it. \a scheme and
	 * \a transport must hold enumerators.
	 *
	 * On success returns std::to_chars_result with ptr past last written character and ec zero. On failure ptr is
	 * \a last and ec is:
	 * - std::errc::invalid_argument if \a host is empty reg-name or !host.is_valid()
	 * - std::errc::value_too_large if output does not fit
	 */
	[[nodiscard]] std::to_chars_result to_chars (char *first, char *last) const noexcept;

	/// Return maximum number of characters to_chars() writes.
	[[nodiscard]] size_t max_size () const noexcept;

	friend constexpr bool operator== (const uri_view &, const uri_view &) noexcept = default;
};

} // namespace turner
