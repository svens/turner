#pragma once

/**
 * \file turner/codec/stun.hpp
 * STUN protocol
 */

#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/value.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace turner
{

/// STUN password algorithm
enum class password_algorithm: uint16_t
{
	md5 = 1, ///< MD5
	sha256 = 2, ///< SHA-256
};

namespace __stun
{

struct algorithm_entry
{
	pal::big_endian<password_algorithm> algorithm;
	pal::big_endian<uint16_t> parameters_length;
};

static_assert(pal::packed_struct<algorithm_entry>);

template <password_algorithm... Valid>
struct basic_password_algorithm_value
{
	using native_type = password_algorithm;

	static constexpr size_t min_size = sizeof(algorithm_entry);
	static constexpr size_t max_size = sizeof(algorithm_entry);

	[[nodiscard]] static size_t size (const native_type &) noexcept
	{
		return max_size;
	}

	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		const auto &wire = pal::packed_as<algorithm_entry>(value.first<max_size>());
		const auto algorithm = wire.algorithm.value();
		if (wire.parameters_length.value() == 0 && __value::one_of<Valid...>(algorithm))
		{
			return algorithm;
		}
		return make_unexpected(errc::unexpected_attribute_value);
	}

	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		pal::packed_as<algorithm_entry>(value.first<max_size>()) =
		{
			.algorithm = native,
			.parameters_length = 0,
		};
	}
};

} // namespace __stun

/// Password algorithm with its parameters length, which is zero for every known algorithm. Only known
/// algorithms with zero parameters length are read successfully.
using password_algorithm_value = __stun::basic_password_algorithm_value<
	password_algorithm::md5,
	password_algorithm::sha256
>;

/// PASSWORD-ALGORITHMS attribute native value: password algorithms in order of preference, at most \ref capacity
/// of them. Algorithms not listed in \ref password_algorithm are kept.
class password_algorithms
{
public:

	/// Maximum number of password algorithms
	static constexpr size_t capacity = 4;

	/// Iterator over password algorithms
	using const_iterator = std::array<password_algorithm, capacity>::const_iterator;

	/// Construct with \a algorithms, at most \ref capacity of them
	template <size_t N>
		requires(N <= capacity)
	constexpr password_algorithms (const password_algorithm (&algorithms)[N]) noexcept
	{
		for (const auto algorithm: algorithms)
		{
			push_back(algorithm);
		}
	}

	/// Return number of password algorithms
	[[nodiscard]] constexpr size_t size () const noexcept
	{
		return size_;
	}

	/// Return iterator to first password algorithm
	[[nodiscard]] constexpr const_iterator begin () const noexcept
	{
		return algorithms_.cbegin();
	}

	/// Return iterator past last password algorithm
	[[nodiscard]] constexpr const_iterator end () const noexcept
	{
		return algorithms_.cbegin() + static_cast<std::ptrdiff_t>(size_);
	}

	/// Return true if \a a and \a b have same password algorithms in same order
	friend constexpr bool operator== (
		const password_algorithms &a,
		const password_algorithms &b
	) noexcept = default;

private:

	std::array<password_algorithm, capacity> algorithms_{};
	size_t size_ = 0;

	constexpr password_algorithms () noexcept = default;

	constexpr void push_back (password_algorithm algorithm) noexcept
	{
		if (size_ < capacity)
		{
			algorithms_[size_++] = algorithm;
		}
	}

	friend struct password_algorithms_value;
};

/// Password algorithms, each with its parameters length. Only lists of at most password_algorithms::capacity
/// algorithms, each with zero parameters length, are read successfully.
struct password_algorithms_value
{
	/// Native value type
	using native_type = password_algorithms;

	/// Minimum value size on wire
	static constexpr size_t min_size = sizeof(__stun::algorithm_entry);

	/// Maximum value size on wire
	static constexpr size_t max_size = password_algorithms::capacity * sizeof(__stun::algorithm_entry);

	/// Return size of \a native on wire
	[[nodiscard]] static size_t size (const native_type &native) noexcept
	{
		return native.size() * sizeof(__stun::algorithm_entry);
	}

	/// Read native value from \a value
	[[nodiscard]] static pal::result<native_type> read (
		std::span<const std::byte>,
		std::span<const std::byte> value) noexcept
	{
		constexpr auto entry_size = sizeof(__stun::algorithm_entry);
		if (value.size() % entry_size != 0)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}

		native_type native;
		for (auto rest = value; !rest.empty(); rest = rest.subspan(entry_size))
		{
			const auto &entry = pal::packed_as<__stun::algorithm_entry>(rest.first<entry_size>());
			if (entry.parameters_length.value() != 0)
			{
				return make_unexpected(errc::unexpected_attribute_value);
			}
			native.push_back(entry.algorithm.value());
		}
		return native;
	}

	/// Write \a native into \a value
	static void write (std::span<const std::byte>, std::span<std::byte> value, const native_type &native) noexcept
	{
		constexpr auto entry_size = sizeof(__stun::algorithm_entry);
		auto rest = value;
		for (const auto algorithm: native)
		{
			pal::packed_as<__stun::algorithm_entry>(rest.first<entry_size>()) =
			{
				.algorithm = algorithm,
				.parameters_length = 0,
			};
			rest = rest.subspan(entry_size);
		}
	}
};

/**
 * STUN protocol
 *
 * \see https://datatracker.ietf.org/doc/html/rfc8489
 */
struct stun
{
	/// Transaction ID
	using transaction_id = std::array<std::byte, 12>;

	/// Offset of transaction ID from beginning of message
	static constexpr size_t transaction_id_offset = 8;

	/// Message header size. Header length field does not count it.
	static constexpr size_t header_size = 20;

	/// Offset of \ref cookie from beginning of message
	static constexpr size_t cookie_offset = 4;

	/// Magic Cookie
	static constexpr std::array cookie =
	{
		std::byte{0x21}, std::byte{0x12}, std::byte{0xa4}, std::byte{0x42},
	};

	/// Attribute padding boundary
	static constexpr size_t attribute_alignment = 4;

	/// Binding
	static constexpr auto binding = method<
		stun,
		0x001,
		message_class::request_response | message_class::indication
	>;

	/// MAPPED-ADDRESS
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.1
	static constexpr auto mapped_address = attribute<stun, endpoint_value, 0x0001>;

	/// USERNAME
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.3
	static constexpr auto username = attribute<stun, string_value<508>, 0x0006>;

	/// ERROR-CODE
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.8
	static constexpr auto error_code = attribute<stun, error_code_value<>, 0x0009>;

	/// UNKNOWN-ATTRIBUTES
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.13
	static constexpr auto unknown_attributes = attribute<stun, unknown_attributes_value, 0x000a>;

	/// REALM
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.9
	static constexpr auto realm = attribute<stun, string_value<763>, 0x0014>;

	/// NONCE
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.10
	static constexpr auto nonce = attribute<stun, string_value<763>, 0x0015>;

	/// PASSWORD-ALGORITHM
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.12
	static constexpr auto password_algorithm = attribute<stun, password_algorithm_value, 0x001d>;

	/// USERHASH
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.4
	static constexpr auto userhash = attribute<stun, bytes_value<32>, 0x001e>;

	/// XOR-MAPPED-ADDRESS
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.2
	static constexpr auto xor_mapped_address = attribute<stun, xor_endpoint_value, 0x0020>;

	/// PASSWORD-ALGORITHMS
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.11
	static constexpr auto password_algorithms = attribute<stun, password_algorithms_value, 0x8002>;

	/// ALTERNATE-DOMAIN
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.16
	static constexpr auto alternate_domain = attribute<stun, string_value<255>, 0x8003>;

	/// SOFTWARE
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.14
	static constexpr auto software = attribute<stun, string_value<763>, 0x8022>;

	/// ALTERNATE-SERVER
	/// \see https://datatracker.ietf.org/doc/html/rfc8489#section-14.15
	static constexpr auto alternate_server = attribute<stun, endpoint_value, 0x8023>;

	/// Validate message in first \a received bytes of \a buffer. \a buffer is whole I/O buffer: response to the
	/// message is built in it. \a received must not exceed \a buffer size.
	///
	/// Checks message structure only (header, attribute layout), not whether message type or attributes are
	/// known. Fails with errc::incomplete if \a received bytes do not hold whole message. Bytes following
	/// the message are ignored: use parsed_message::size() to find them.
	[[nodiscard]] static pal::result<parsed_message<stun>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;
};

} // namespace turner
