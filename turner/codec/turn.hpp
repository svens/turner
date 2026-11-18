#pragma once

/**
 * \file turner/codec/turn.hpp
 * TURN protocol
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/protocol_error.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/value.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace turner
{

namespace __turn
{

struct channel_number
{
	pal::big_endian<uint16_t> channel;
	std::array<std::byte, 2> reserved;
};

static_assert(pal::packed_struct<channel_number>);

} // namespace __turn

/// Channel number: 16-bit unsigned integer followed by two reserved bytes. Only numbers 0x4000 to 0x4fff are
/// read successfully; reserved bytes are ignored on read and written as zero.
struct channel_number_value
{
	/// Native value type
	using native_type = uint16_t;

	/// Minimum value size on wire
	static constexpr size_t min_size = sizeof(__turn::channel_number);

	/// Maximum value size on wire
	static constexpr size_t max_size = sizeof(__turn::channel_number);

	/// First valid channel number
	static constexpr native_type first = 0x4000;

	/// Last valid channel number
	static constexpr native_type last = 0x4fff;

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return max_size;
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		const auto channel = pal::packed_as<__turn::channel_number>(value.first<max_size>()).channel.value();
		if (first <= channel && channel <= last)
		{
			return channel;
		}
		return make_unexpected(errc::unexpected_attribute_value);
	}

	/// Write \a native into \a value. Channel number must be in range \ref first to \ref last.
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		pal::packed_as<__turn::channel_number>(value.first<max_size>()) = {.channel = native, .reserved = {}};
	}
};

/// ADDRESS-ERROR-CODE attribute native value
struct address_error
{
	/// Address family that was not allocated
	address_family family;

	/// Reason it was not allocated
	protocol_error error;
};

/// Address family followed by error code laid out as error_code_value, with the address family in place of
/// its first reserved byte
struct address_error_code_value
{
	/// Native value type
	using native_type = address_error;

	/// Minimum value size on wire
	static constexpr size_t min_size = error_code_value<>::min_size;

	/// Maximum value size on wire
	static constexpr size_t max_size = error_code_value<>::max_size;

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return error_code_value<>::size(native.error);
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte> message,
		std::span<const std::byte> value) noexcept
	{
		const auto family = address_family_value::read(message, value.first<address_family_value::max_size>());
		return family.and_then([&](address_family valid) noexcept
		{
			const auto error = error_code_value<>::read(message, value);
			return error.transform([valid](const protocol_error &code) noexcept
			{
				return native_type{.family = valid, .error = code};
			});
		});
	}

	/// Write \a native into \a value. Error code must be in range 300 to 699.
	static void write (
		std::span<const std::byte> message,
		std::span<std::byte> value,
		const native_type &native) noexcept
	{
		error_code_value<>::write(message, value, native.error);
		value[0] = static_cast<std::byte>(native.family);
	}
};

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

	/// CHANNEL-NUMBER
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.1
	static constexpr auto channel_number = attribute<turn, channel_number_value, 0x000c>;

	/// LIFETIME
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.2
	static constexpr auto lifetime = attribute<turn, seconds_value, 0x000d>;

	/// XOR-PEER-ADDRESS
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.3
	static constexpr auto xor_peer_address = attribute<turn, xor_endpoint_value, 0x0012>;

	/// DATA
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.4
	static constexpr auto data = attribute<turn, bytes_value<>, 0x0013>;

	/// XOR-RELAYED-ADDRESS
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.5
	static constexpr auto xor_relayed_address = attribute<turn, xor_endpoint_value, 0x0016>;

	/// REQUESTED-ADDRESS-FAMILY
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.6
	static constexpr auto requested_address_family = attribute<turn, address_family_value, 0x0017>;

	/// REQUESTED-TRANSPORT
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.8
	static constexpr auto requested_transport = attribute<turn, transport_protocol_value, 0x0019>;

	/// DONT-FRAGMENT
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.9
	static constexpr auto dont_fragment = attribute<turn, flag_value, 0x001a>;

	/// ADDITIONAL-ADDRESS-FAMILY
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.11
	static constexpr auto additional_address_family = attribute<turn, address_family_value, 0x8000>;

	/// ADDRESS-ERROR-CODE
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-18.12
	static constexpr auto address_error_code = attribute<turn, address_error_code_value, 0x8001>;

	/// \copydoc stun::parse
	[[nodiscard]] static pal::result<parsed_message<turn>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;

	/// Most bytes make_data_indication() and make_channel_data() write before payload
	static constexpr size_t max_headroom =
		header_size
		+ __wire::attribute_header_size + decltype(xor_peer_address)::value_type::max_size
		+ __wire::attribute_header_size;

	/// Most bytes make_data_indication() and make_channel_data() write after payload
	static constexpr size_t max_tailroom = attribute_alignment - 1;

	/// Largest payload that length fields of make_data_indication() and make_channel_data() express for every
	/// peer address family
	static constexpr size_t max_payload = (0xffff - (max_headroom - header_size)) & ~(attribute_alignment - 1);

	/// Return what \a bytes, which hold only received bytes, start with, by first byte: STUN or TURN message,
	/// ChannelData or other. Nothing is validated: use parse() or parse_channel_data() next. Empty \a bytes is
	/// content::need_more. On stream, content::other cannot be skipped, so session cannot continue.
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-12
	[[nodiscard]] static content peek (std::span<const std::byte> bytes) noexcept;

	/// Validated ChannelData message. Created by parse_channel_data().
	class channel_data
	{
	public:

		/// Return channel number
		[[nodiscard]] uint16_t channel () const noexcept
		{
			return channel_;
		}

		/// Return application data. It refers to bytes of buffer given to parse_channel_data().
		[[nodiscard]] std::span<const std::byte> payload () const noexcept
		{
			return payload_;
		}

		/// Return number of received bytes the message occupies, padding included. It never exceeds bytes
		/// received.
		[[nodiscard]] size_t size () const noexcept
		{
			return size_;
		}

	private:

		std::span<const std::byte> payload_;
		size_t size_;
		uint16_t channel_;

		channel_data (uint16_t channel, std::span<const std::byte> payload, size_t size) noexcept
			: payload_{payload}
			, size_{size}
			, channel_{channel}
		{
		}

		friend turn;
	};

	/// Validate ChannelData message in first \a received bytes of \a buffer. \a received must not exceed \a buffer
	/// size. Fails with errc::unexpected_message_type if channel number is not valid, and with errc::incomplete if
	/// \a received bytes do not hold whole message. Padding to multiple of 4 bytes is part of message on
	/// transport::stream and optional on transport::datagram. Bytes following the message are ignored: use
	/// channel_data::size() to find them.
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-12.6
	[[nodiscard]] static pal::result<channel_data> parse_channel_data (
		std::span<const std::byte> buffer,
		size_t received,
		turner::transport transport
	) noexcept;

	/// Make ChannelData message of \a channel around \a payload in \a buffer and return it, padded to multiple of
	/// 4 bytes. \a payload must refer to bytes of \a buffer, also when empty, be at most \ref max_payload bytes
	/// and have 4 bytes before and \ref max_tailroom bytes after it in \a buffer. \a channel must be valid. These
	/// are not checked.
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-12.5
	[[nodiscard]] static std::span<const std::byte> make_channel_data (
		std::span<std::byte> buffer,
		std::span<const std::byte> payload,
		uint16_t channel
	) noexcept;

	/// Make Data indication from \a peer with \a id around \a payload in \a buffer and return it. \a payload must
	/// refer to bytes of \a buffer, also when empty, be at most \ref max_payload bytes and have \ref max_headroom
	/// bytes before and \ref max_tailroom bytes after it in \a buffer. \a peer and \a id must not refer to bytes
	/// of \a buffer. These are not checked.
	/// \see https://datatracker.ietf.org/doc/html/rfc8656#section-11.3
	[[nodiscard]] static std::span<const std::byte> make_data_indication (
		std::span<std::byte> buffer,
		std::span<const std::byte> payload,
		const transport_address &peer,
		const transaction_id &id
	) noexcept;
};

} // namespace turner
