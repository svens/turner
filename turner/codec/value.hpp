#pragma once

/**
 * \file turner/codec/value.hpp
 * Attribute value types
 */

#include <turner/codec/error.hpp>
#include <turner/codec/protocol_error.hpp>
#include <pal/net/ip/address.hpp>
#include <pal/net/ip/address_v4.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <pal/net/ip/basic_endpoint.hpp>
#include <pal/net/ip/port_type.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

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

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return sizeof(T);
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		return pal::packed_as<pal::big_endian<T>>(value.first<sizeof(T)>()).value();
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

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return uint32_value::max_size;
	}

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

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.size();
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		return native_type{reinterpret_cast<const char *>(value.data()), value.size()};
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

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.size();
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		return native_type{value.data(), value.size()};
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

	/// Return 0
	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return 0;
	}

	/// Return \c true
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte>) noexcept
	{
		return native_type{};
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
		const auto native = static_cast<Enum>(value[0]);
		if (((native == Valid) || ...))
		{
			return native;
		}
		return make_unexpected(errc::unexpected_attribute_value);
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

/// IP address and port
struct transport_address
{
	/// IP address
	pal::net::ip::address address;

	/// Port
	pal::net::ip::port_type port;

	/// Construct with \a ip and \a number
	constexpr transport_address (const pal::net::ip::address &ip, pal::net::ip::port_type number) noexcept
		: address{ip}
		, port{number}
	{
	}

	/// Construct with address and port of \a endpoint
	template <typename Protocol>
	constexpr transport_address (const pal::net::ip::basic_endpoint<Protocol> &endpoint) noexcept
		: address{endpoint.address()}
		, port{endpoint.port()}
	{
	}

	/// Return true if \a a and \a b have same address and port
	friend constexpr bool operator== (const transport_address &a, const transport_address &b) noexcept = default;
};

namespace __value
{

template <typename Address>
struct endpoint
{
	std::byte reserved;
	address_family family;
	pal::big_endian<pal::net::ip::port_type> port;
	typename Address::bytes_type address;
};

static_assert(pal::packed_struct<endpoint<pal::net::ip::address_v4>>);
static_assert(pal::packed_struct<endpoint<pal::net::ip::address_v6>>);

template <typename Address>
constexpr address_family family_of = std::same_as<Address, pal::net::ip::address_v4>
	? address_family::v4
	: address_family::v6
;

using xor_key_type = std::span<const std::byte, 16>;

struct zero_key
{
	static constexpr std::array<std::byte, 16> zero{};

	static xor_key_type get (std::span<const std::byte>) noexcept
	{
		return zero;
	}
};

struct message_key
{
	static xor_key_type get (std::span<const std::byte> message) noexcept
	{
		return message.subspan<4, 16>();
	}
};

inline pal::net::ip::port_type xor_with (pal::net::ip::port_type port, xor_key_type key) noexcept
{
	const auto mask = pal::packed_as<pal::big_endian<uint16_t>>(key.first<sizeof(uint16_t)>()).value();
	return static_cast<pal::net::ip::port_type>(std::to_underlying(port) ^ mask);
}

template <typename Bytes>
Bytes xor_with (const Bytes &bytes, xor_key_type key) noexcept
{
	Bytes result{};
	std::ranges::transform(bytes, key, result.begin(), [](uint8_t byte, std::byte mask) noexcept
	{
		return static_cast<uint8_t>(byte ^ std::to_integer<uint8_t>(mask));
	});
	return result;
}

template <typename Key>
struct basic_endpoint_value
{
	using native_type = transport_address;

	static constexpr size_t min_size = sizeof(endpoint<pal::net::ip::address_v4>);
	static constexpr size_t max_size = sizeof(endpoint<pal::net::ip::address_v6>);

	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.address.is_v4() ? min_size : max_size;
	}

	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte> message,
		std::span<const std::byte> value) noexcept
	{
		if (value.size() == min_size)
		{
			return decode<pal::net::ip::address_v4>(message, value.first<min_size>());
		}
		if (value.size() == max_size)
		{
			return decode<pal::net::ip::address_v6>(message, value.first<max_size>());
		}
		return make_unexpected(errc::unexpected_attribute_length);
	}

	static void write (
		std::span<const std::byte> message,
		std::span<std::byte> value,
		const native_type &native) noexcept
	{
		if (const auto *v4 = native.address.v4())
		{
			encode(message, value.first<min_size>(), *v4, native.port);
		}
		else
		{
			encode(message, value.first<max_size>(), *native.address.v6(), native.port);
		}
	}

private:

	template <typename Address>
	[[nodiscard]] static pal::result<native_type> decode (
		std::span<const std::byte> message,
		std::span<const std::byte, sizeof(endpoint<Address>)> value) noexcept
	{
		const auto &wire = pal::packed_as<endpoint<Address>>(value);
		if (wire.family != family_of<Address>)
		{
			return make_unexpected(errc::unexpected_attribute_value);
		}
		const auto key = Key::get(message);
		return native_type{Address{xor_with(wire.address, key)}, xor_with(wire.port.value(), key)};
	}

	template <typename Address>
	static void encode (
		std::span<const std::byte> message,
		std::span<std::byte, sizeof(endpoint<Address>)> value,
		const Address &address,
		pal::net::ip::port_type port) noexcept
	{
		const auto key = Key::get(message);
		pal::packed_as<endpoint<Address>>(value) =
		{
			.reserved = {},
			.family = family_of<Address>,
			.port = xor_with(port, key),
			.address = xor_with(address.to_bytes(), key),
		};
	}
};

} // namespace __value

/// IP address and port: reserved byte, address family, port, address. Reserved byte is ignored on read and
/// written as zero. IPv6 scope ID is not on wire: it is read as zero and ignored on write.
using endpoint_value = __value::basic_endpoint_value<__value::zero_key>;

/// As endpoint_value, with port and address XORed with message bytes 4 to 19: STUN Magic Cookie and
/// transaction ID, MS-TURN transaction ID.
using xor_endpoint_value = __value::basic_endpoint_value<__value::message_key>;

/// Error code with reason phrase of at most \a MaxReason bytes: 21 reserved bits, class (3 bits), number
/// (8 bits), reason. Reserved bits are ignored on read and written as zero. Codes of class 3 to 6 with number
/// 0 to 99 are read successfully, known or not. Reason refers to bytes in message; content is not validated.
template <size_t MaxReason = 763>
struct error_code_value
{
	/// Native value type
	using native_type = protocol_error;

	/// Minimum value size on wire
	static constexpr size_t min_size = 4;

	/// Maximum value size on wire
	static constexpr size_t max_size = min_size + MaxReason;

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return min_size + native.reason.size();
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte> message,
		std::span<const std::byte> value) noexcept
	{
		const auto error_class = std::to_integer<uint16_t>(value[2] & std::byte{0b111});
		const auto number = std::to_integer<uint16_t>(value[3]);
		if (error_class < 3 || error_class > 6 || number > 99)
		{
			return make_unexpected(errc::unexpected_attribute_value);
		}

		const auto code = static_cast<protocol_errc>(error_class * 100 + number);
		const auto reason = reason_value::read(message, value.subspan(min_size));
		return reason.transform([code](std::string_view phrase) noexcept
		{
			return native_type{code, phrase};
		});
	}

	/// Write \a native into \a value. Code must be in range 300 to 699.
	static void write (
		std::span<const std::byte> message,
		std::span<std::byte> value,
		const native_type &native) noexcept
	{
		const auto code = std::to_underlying(native.code);
		value[0] = std::byte{};
		value[1] = std::byte{};
		value[2] = static_cast<std::byte>(code / 100);
		value[3] = static_cast<std::byte>(code % 100);
		reason_value::write(message, value.subspan(min_size), native.reason);
	}

private:

	using reason_value = string_value<MaxReason>;
};

/// UNKNOWN-ATTRIBUTES attribute native value: attribute types, at most \ref capacity of them
class unknown_attributes
{
public:

	/// Maximum number of attribute types
	static constexpr size_t capacity = 4;

	/// Iterator over attribute types
	using const_iterator = std::array<uint16_t, capacity>::const_iterator;

	/// Construct with no attribute types
	constexpr unknown_attributes () noexcept = default;

	/// Construct with \a types, at most \ref capacity of them
	template <size_t N>
		requires(N <= capacity)
	constexpr unknown_attributes (const uint16_t (&types)[N]) noexcept
	{
		for (const auto type: types)
		{
			push_back(type);
		}
	}

	/// Append \a type. Dropped if \ref capacity is reached.
	constexpr void push_back (uint16_t type) noexcept
	{
		if (size_ < capacity)
		{
			types_[size_++] = type;
		}
	}

	/// Return number of attribute types
	[[nodiscard]] constexpr size_t size () const noexcept
	{
		return size_;
	}

	/// Return iterator to first attribute type
	[[nodiscard]] constexpr const_iterator begin () const noexcept
	{
		return types_.cbegin();
	}

	/// Return iterator past last attribute type
	[[nodiscard]] constexpr const_iterator end () const noexcept
	{
		return types_.cbegin() + static_cast<std::ptrdiff_t>(size_);
	}

	/// Return true if there is at least one attribute type
	constexpr explicit operator bool () const noexcept
	{
		return size_ != 0;
	}

	/// Return true if \a a and \a b have same attribute types in same order
	friend constexpr bool operator== (const unknown_attributes &a, const unknown_attributes &b) noexcept = default;

private:

	std::array<uint16_t, capacity> types_{};
	size_t size_ = 0;
};

/// Attribute types in network byte order. Types beyond unknown_attributes::capacity are dropped on read.
struct unknown_attributes_value
{
	/// Native value type
	using native_type = unknown_attributes;

	/// Minimum value size on wire
	static constexpr size_t min_size = sizeof(uint16_t);

	/// Maximum value size on wire
	static constexpr size_t max_size = 0xfffe;

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.size() * sizeof(uint16_t);
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		if (value.size() % sizeof(uint16_t) != 0)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}

		native_type native;
		const auto kept = (std::min)(value.size(), unknown_attributes::capacity * sizeof(uint16_t));
		for (auto rest = value.first(kept); !rest.empty(); rest = rest.subspan(sizeof(uint16_t)))
		{
			const auto &type = pal::packed_as<pal::big_endian<uint16_t>>(rest.first<sizeof(uint16_t)>());
			native.push_back(type.value());
		}
		return native;
	}

	/// Write \a native into \a value
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		auto rest = value;
		for (const auto type: native)
		{
			pal::packed_as<pal::big_endian<uint16_t>>(rest.first<sizeof(uint16_t)>()) = type;
			rest = rest.subspan(sizeof(uint16_t));
		}
	}
};

} // namespace turner
