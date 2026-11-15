#pragma once

/**
 * \file turner/codec/value.hpp
 * Attribute value types
 */

#include <turner/codec/error.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

namespace turner
{

/// Unsigned integer \a T in network byte order
template <std::unsigned_integral T>
	requires(!std::same_as<T, bool>)
struct integer_value
{
	/// Native value type
	using native_type = T;

	/// Minimum value size on wire
	static constexpr size_t min_size = sizeof(T);

	/// Maximum value size on wire
	static constexpr size_t max_size = sizeof(T);

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		return pal::packed_as<pal::big_endian<T>>(value.first<sizeof(T)>()).value();
	}

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return sizeof(T);
	}

	/// Write \a native into \a value
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		pal::packed_as<pal::big_endian<T>>(value.first<sizeof(T)>()) = native;
	}
};

/// 32-bit unsigned integer
using uint32_value = integer_value<uint32_t>;

/// Duration in seconds as 32-bit unsigned integer
struct seconds_value
{
	/// Native value type
	using native_type = std::chrono::seconds;

	/// Minimum value size on wire
	static constexpr size_t min_size = uint32_value::min_size;

	/// Maximum value size on wire
	static constexpr size_t max_size = uint32_value::max_size;

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte> message,
		std::span<const std::byte> value) noexcept
	{
		return uint32_value::read(message, value).transform([](uint32_t count) noexcept
		{
			return native_type{count};
		});
	}

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return uint32_value::max_size;
	}

	/// Write \a native into \a value. Negative duration is written as zero, duration not representable as
	/// 32-bit unsigned integer as its maximum.
	static void write (
		std::span<const std::byte> message,
		std::span<std::byte> value,
		const native_type &native) noexcept
	{
		constexpr native_type::rep max = 0xffff'ffff;
		const auto count = std::clamp<native_type::rep>(native.count(), 0, max);
		uint32_value::write(message, value, static_cast<uint32_t>(count));
	}
};

/// Text of at most \a MaxSize bytes. Native value refers to bytes in message; content is not validated.
template <size_t MaxSize = 0xffff>
struct string_value
{
	/// Native value type
	using native_type = std::string_view;

	/// Minimum value size on wire
	static constexpr size_t min_size = 0;

	/// Maximum value size on wire
	static constexpr size_t max_size = MaxSize;

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		return native_type{reinterpret_cast<const char *>(value.data()), value.size()};
	}

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.size();
	}

	/// Write \a native into \a value
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		std::ranges::copy(std::as_bytes(std::span{native}), value.begin());
	}
};

/// Opaque bytes: exactly \a Extent of them, or any number if \a Extent is \c std::dynamic_extent. Native
/// value refers to bytes in message.
template <size_t Extent = std::dynamic_extent>
struct bytes_value
{
	/// Native value type
	using native_type = std::span<const std::byte, Extent>;

	/// Minimum value size on wire
	static constexpr size_t min_size = Extent == std::dynamic_extent ? 0 : Extent;

	/// Maximum value size on wire
	static constexpr size_t max_size = Extent == std::dynamic_extent ? 0xffff : Extent;

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		return native_type{value.data(), value.size()};
	}

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.size();
	}

	/// Write \a native into \a value
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		std::ranges::copy(native, value.begin());
	}
};

/// Attribute without value: its presence is the value. Native type has single value, \c true, so result of
/// reading the attribute tells only whether it is present.
struct flag_value
{
	/// Native value type
	using native_type = std::true_type;

	/// Minimum value size on wire
	static constexpr size_t min_size = 0;

	/// Maximum value size on wire
	static constexpr size_t max_size = 0;

	/// Return \c true
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte>) noexcept
	{
		return native_type{};
	}

	/// Return 0
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return 0;
	}

	/// Write nothing
	static void write (std::span<const std::byte>, std::span<std::byte>, const native_type &) noexcept
	{
	}
};

/// One byte enumeration \a Enum followed by three reserved bytes. Only \a Valid values are read successfully.
template <typename Enum, Enum... Valid>
	requires(sizeof(Enum) == 1)
struct enum_value
{
	/// Native value type
	using native_type = Enum;

	/// Minimum value size on wire
	static constexpr size_t min_size = 4;

	/// Maximum value size on wire
	static constexpr size_t max_size = 4;

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		const auto native = static_cast<Enum>(value[0]);
		if (((native == Valid) || ...))
		{
			return native;
		}
		return make_unexpected(errc::unexpected_attribute_value);
	}

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return max_size;
	}

	/// Write \a native into \a value
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		std::ranges::fill(value.first(max_size), std::byte{});
		value[0] = static_cast<std::byte>(native);
	}
};

/// Address family
enum class address_family: uint8_t
{
	v4 = 1, ///< IPv4
	v6 = 2, ///< IPv6
};

/// Address family value
using address_family_value = enum_value<address_family,
	address_family::v4,
	address_family::v6
>;

/// Transport protocol
/// \see https://www.iana.org/assignments/protocol-numbers
enum class transport_protocol: uint8_t
{
	tcp = 6, ///< TCP
	udp = 17, ///< UDP
};

/// Transport protocol value
using transport_protocol_value = enum_value<transport_protocol,
	transport_protocol::tcp,
	transport_protocol::udp
>;

} // namespace turner
