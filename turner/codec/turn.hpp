#pragma once

/**
 * \file turner/codec/turn.hpp
 * TURN protocol
 */

#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/method.hpp>
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
};

} // namespace turner
