#pragma once

/**
 * \file turner/codec/msturn.hpp
 * MS-TURN protocol
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol_error.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/value.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

namespace turner
{

struct msturn;

namespace __method
{

template <>
inline constexpr uint16_t wire_type<msturn, 0x005, message_class::indication> = 0x0115;

} // namespace __method

namespace __msturn
{

inline constexpr size_t value_alignment = 4;

using error_code_value = turner::error_code_value<
	0xffff - turner::error_code_value<>::min_size - (value_alignment - 1)
>;

struct service_quality
{
	pal::big_endian<uint16_t> stream;
	pal::big_endian<uint16_t> quality;
};

static_assert(pal::packed_struct<service_quality>);

struct sequence_number
{
	std::array<std::byte, 20> connection_id;
	pal::big_endian<uint32_t> sequence;
};

static_assert(pal::packed_struct<sequence_number>);

} // namespace __msturn

/**
 * MS-TURN protocol
 *
 * \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn
 */
struct msturn
{
	/// Transaction ID
	using transaction_id = std::array<std::byte, 16>;

	/// Offset of transaction ID from beginning of message
	static constexpr size_t transaction_id_offset = 4;

	/// Message header size. Header length field does not count it.
	static constexpr size_t header_size = 20;

	/// Offset of \ref cookie from beginning of message
	static constexpr size_t cookie_offset = header_size;

	/// Magic Cookie: attribute following message header, with its type and length fields
	static constexpr std::array cookie =
	{
		std::byte{0x00}, std::byte{0x0f}, std::byte{0x00}, std::byte{0x04},
		std::byte{0x72}, std::byte{0xc6}, std::byte{0x4b}, std::byte{0xc6},
	};

	/// Attribute padding boundary
	static constexpr size_t attribute_alignment = 1;

	/// Allocate
	static constexpr auto allocate = request<msturn, 0x003>;

	/// Send Request: request without responses
	static constexpr auto send_request = method<msturn, 0x004, message_class::request>;

	/// Data Indication
	static constexpr auto data_indication = indication<msturn, 0x005>;

	/// Set Active Destination
	static constexpr auto set_active_destination = request<msturn, 0x006>;

	/// Error code as turner::error_code_value with reason of any length. On write, reason is padded with spaces
	/// to multiple of 4 bytes; on read, reason is returned as is, padding included.
	struct error_code_value
	{
		/// Native value type
		using native_type = protocol_error;

		/// Minimum value size on wire
		static constexpr size_t min_size = __msturn::error_code_value::min_size;

		/// Maximum value size on wire
		static constexpr size_t max_size = __msturn::error_code_value::max_size;

		/// Return size of \a native on wire
		[[nodiscard]] static size_t size (const native_type &native) noexcept
		{
			return __wire::align_up(__msturn::error_code_value::size(native), __msturn::value_alignment);
		}

		/// Read native value from \a value
		[[nodiscard]] static pal::result<native_type> read (
			std::span<const std::byte> message,
			std::span<const std::byte> value) noexcept
		{
			return __msturn::error_code_value::read(message, value);
		}

		/// Write \a native into \a value. Error code must be in range 300 to 699.
		static void write (
			std::span<const std::byte> message,
			std::span<std::byte> value,
			const native_type &native) noexcept
		{
			const auto unpadded = __msturn::error_code_value::size(native);
			__msturn::error_code_value::write(message, value.first(unpadded), native);
			std::ranges::fill(value.subspan(unpadded, size(native) - unpadded), std::byte{' '});
		}
	};

	/// Attribute types as turner::unknown_attributes_value. On write, last type is repeated when the number of
	/// types is odd; on read, types are returned as is, repeated type included.
	struct unknown_attributes_value
	{
		/// Native value type
		using native_type = attribute_type_list;

		/// Minimum value size on wire
		static constexpr size_t min_size = turner::unknown_attributes_value::min_size;

		/// Maximum value size on wire
		static constexpr size_t max_size = turner::unknown_attributes_value::max_size;

		/// Return size of \a native on wire
		[[nodiscard]] static size_t size (const native_type &native) noexcept
		{
			const auto unpadded = turner::unknown_attributes_value::size(native);
			return __wire::align_up(unpadded, __msturn::value_alignment);
		}

		/// Read native value from \a value
		[[nodiscard]] static pal::result<native_type> read (
			std::span<const std::byte> message,
			std::span<const std::byte> value) noexcept
		{
			return turner::unknown_attributes_value::read(message, value);
		}

		/// Write \a native into \a value
		static void write (
			std::span<const std::byte> message,
			std::span<std::byte> value,
			const native_type &native) noexcept
		{
			const auto unpadded = turner::unknown_attributes_value::size(native);
			turner::unknown_attributes_value::write(message, value.first(unpadded), native);
			if (unpadded != size(native))
			{
				const auto repeated = value.subspan(unpadded).first<sizeof(uint16_t)>();
				pal::packed_as<pal::big_endian<uint16_t>>(repeated) = *(native.end() - 1);
			}
		}
	};

	/// TURN protocol version. Each version includes features of previous ones: test feature support with
	/// \c >= against feature-named alias.
	enum class protocol_version: uint32_t
	{
		v1 = 1, ///< ICE as in MS-ICE
		with_ice = v1, ///< \copydoc v1

		v2 = 2, ///< ICE as in MS-ICE2
		with_ice2 = v2, ///< \copydoc v2

		v3 = 3, ///< v2 with HMAC SHA-256 in Message Integrity
		with_sha256 = v3, ///< \copydoc v3

		v4 = 4, ///< v3 with IPv6 addresses
		with_ipv6 = v4, ///< \copydoc v4

		v5 = 5, ///< v4 with Multiplexed TURN over UDP
		with_mturn_udp = v5, ///< \copydoc v5

		v6 = 6, ///< v5 with Multiplexed TURN over TCP
		with_mturn_tcp = v6, ///< \copydoc v6
	};

	/// TURN protocol version as 32-bit unsigned integer. Every value is read successfully, including versions
	/// not listed in \ref protocol_version.
	struct protocol_version_value
	{
		/// Native value type
		using native_type = protocol_version;

		/// Minimum value size on wire
		static constexpr size_t min_size = sizeof(native_type);

		/// Maximum value size on wire
		static constexpr size_t max_size = sizeof(native_type);

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
			return pal::packed_as<pal::big_endian<native_type>>(value.first<max_size>()).value();
		}

		/// Write \a native into \a value
		static void write (
			std::span<const std::byte>,
			std::span<std::byte> value,
			const native_type &native) noexcept
		{
			pal::packed_as<pal::big_endian<native_type>>(value.first<max_size>()) = native;
		}
	};

	/// Type of data stream
	enum class stream_type: uint16_t
	{
		audio = 1, ///< Audio
		video = 2, ///< Video
		supplemental_video = 3, ///< Supplemental video
		data = 4, ///< Data
	};

	/// Delivery quality of data stream
	enum class delivery: uint16_t
	{
		best_effort = 0, ///< Best effort
		reliable = 1, ///< Reliable
	};

	/// MS-SERVICE-QUALITY attribute native value
	struct service_quality
	{
		/// Type of data stream
		stream_type stream;

		/// Delivery quality required for the stream
		delivery quality;

		/// Return true if \a a and \a b have same stream type and delivery quality
		friend constexpr bool operator== (
			const service_quality &a,
			const service_quality &b
		) noexcept = default;
	};

	/// Stream type and delivery quality as 16-bit unsigned integers. Every value is read successfully,
	/// including ones not listed in \ref stream_type and \ref delivery.
	struct service_quality_value
	{
		/// Native value type
		using native_type = service_quality;

		/// Minimum value size on wire
		static constexpr size_t min_size = sizeof(__msturn::service_quality);

		/// Maximum value size on wire
		static constexpr size_t max_size = sizeof(__msturn::service_quality);

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
			const auto &wire = pal::packed_as<__msturn::service_quality>(value.first<max_size>());
			return native_type
			{
				.stream = static_cast<stream_type>(wire.stream.value()),
				.quality = static_cast<delivery>(wire.quality.value()),
			};
		}

		/// Write \a native into \a value
		static void write (
			std::span<const std::byte>,
			std::span<std::byte> value,
			const native_type &native) noexcept
		{
			pal::packed_as<__msturn::service_quality>(value.first<max_size>()) =
			{
				.stream = std::to_underlying(native.stream),
				.quality = std::to_underlying(native.quality),
			};
		}
	};

	/// MS-SEQUENCE-NUMBER attribute native value
	struct sequence_number
	{
		/// Connection ID generated by TURN server
		decltype(__msturn::sequence_number::connection_id) connection_id;

		/// Sequence number incremented by client for each request
		uint32_t sequence;

		/// Return true if \a a and \a b have same connection ID and sequence number
		friend constexpr bool operator== (
			const sequence_number &a,
			const sequence_number &b
		) noexcept = default;
	};

	/// Connection ID followed by sequence number as 32-bit unsigned integer
	struct sequence_number_value
	{
		/// Native value type
		using native_type = sequence_number;

		/// Minimum value size on wire
		static constexpr size_t min_size = sizeof(__msturn::sequence_number);

		/// Maximum value size on wire
		static constexpr size_t max_size = sizeof(__msturn::sequence_number);

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
			const auto &wire = pal::packed_as<__msturn::sequence_number>(value.first<max_size>());
			return native_type{.connection_id = wire.connection_id, .sequence = wire.sequence.value()};
		}

		/// Write \a native into \a value
		static void write (
			std::span<const std::byte>,
			std::span<std::byte> value,
			const native_type &native) noexcept
		{
			pal::packed_as<__msturn::sequence_number>(value.first<max_size>()) =
			{
				.connection_id = native.connection_id,
				.sequence = native.sequence,
			};
		}
	};

	/// IP address and port as endpoint_value, IPv6 only: IPv4 address is below \c min_size, so reader and
	/// writer reject it
	struct ipv6_endpoint_value: endpoint_value
	{
		/// Minimum value size on wire
		static constexpr size_t min_size = endpoint_value::max_size;
	};

	/// Mapped Address
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/735464a2-28e3-423f-8b65-19dd4b4dbc52
	static constexpr auto mapped_address = attribute<msturn, endpoint_value, 0x0001>;

	/// Username
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/bbf7bc45-93b6-495e-adcc-685d270a2eae
	static constexpr auto username = attribute<msturn, string_value<>, 0x0006>;

	/// Error Code
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/91972889-8dfd-43d1-885e-7d135bd91c2d
	static constexpr auto error_code = attribute<msturn, error_code_value, 0x0009>;

	/// Unknown Attributes
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/2b097c56-f250-43fe-970e-66ca08b2dfcd
	static constexpr auto unknown_attributes = attribute<msturn, unknown_attributes_value, 0x000a>;

	/// Lifetime
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/46555d94-a165-40c9-a5ff-4d841d4d012a
	static constexpr auto lifetime = attribute<msturn, seconds_value, 0x000d>;

	/// Alternate Server
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/c8ebc611-c0c4-48c4-b7da-47e57b3f96c3
	static constexpr auto alternate_server = attribute<msturn, endpoint_value, 0x000e>;

	/// Bandwidth
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/f98b6d22-e8e2-4817-b936-5904402d1788
	static constexpr auto bandwidth = attribute<msturn, uint32_value, 0x0010>;

	/// Destination Address
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/9cd7e664-7c19-4464-ab05-761b4ca39372
	static constexpr auto destination_address = attribute<msturn, endpoint_value, 0x0011>;

	/// Remote Address
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/cf5c3756-d0a7-414e-8a14-c54786e34dbc
	static constexpr auto remote_address = attribute<msturn, endpoint_value, 0x0012>;

	/// Data
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/d9dd90bc-5bd3-4daa-89ac-0d70c5a1cb81
	static constexpr auto data = attribute<msturn, bytes_value<>, 0x0013>;

	/// Nonce
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/129e9c99-7d4b-4c8c-abd0-8d7a5874d73d
	static constexpr auto nonce = attribute<msturn, string_value<128>, 0x0014>;

	/// Realm
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/108c58d4-129d-4ba5-b12b-fc6c1a54e7b3
	static constexpr auto realm = attribute<msturn, string_value<128>, 0x0015>;

	/// Requested Address Family
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/e7efc457-9312-4a6b-8089-94032a599198
	static constexpr auto requested_address_family = attribute<msturn, address_family_value, 0x0017>;

	/// MS-Version
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/a972c937-780a-476a-915a-b81869428342
	static constexpr auto ms_version = attribute<msturn, protocol_version_value, 0x8008>;

	/// XOR Mapped Address
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/d6f3f10a-b5f2-423a-af1d-a1d69b09ddab
	static constexpr auto xor_mapped_address = attribute<msturn, xor_endpoint_value, 0x8020>;

	/// MS-Alternate Host Name
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/69ca1e1a-7423-4a3a-8618-f0bf76383cdc
	static constexpr auto ms_alternate_host_name = attribute<msturn, string_value<128>, 0x8032>;

	/// APP-ID: application identifier, for diagnostics
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/0a4bffc9-6cdc-4525-b76d-34a3606a7603
	static constexpr auto app_id = attribute<msturn, uint32_value, 0x8037>;

	/// SECURE-TAG: APP-ID identifier, salt and HMAC-SHA1
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/571bc09c-81b7-4e53-a64b-1b1c2278b7cb
	static constexpr auto secure_tag = attribute<msturn, bytes_value<40>, 0x8039>;

	/// MS-Sequence Number
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/bf1907a9-2135-41fb-8316-5988f1438ab1
	static constexpr auto ms_sequence_number = attribute<msturn, sequence_number_value, 0x8050>;

	/// MS-Service Quality
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/e6add235-93a1-4802-b05a-1dabb1544344
	static constexpr auto ms_service_quality = attribute<msturn, service_quality_value, 0x8055>;

	/// MS-Alternate Mapped Address
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/3a99a9cf-7757-4d9a-87b6-688bcfe15f10
	static constexpr auto ms_alternate_mapped_address = attribute<msturn, ipv6_endpoint_value, 0x8090>;

	/// Multiplexed TURN Session ID
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/8192de42-81aa-453b-b4f4-77d4889da70e
	static constexpr auto multiplexed_session_id = attribute<msturn, bytes_value<8>, 0x8095>;

	/// Validate message in first \a received bytes of \a buffer. \a buffer is whole I/O buffer: response to the
	/// message is built in it. \a received must not exceed \a buffer size.
	///
	/// Checks message structure only (header, Magic Cookie, attribute layout), not whether message type or
	/// attributes are known. Fails with errc::incomplete if \a received bytes do not hold whole message.
	/// Bytes following the message are ignored: use parsed_message::size() to find them.
	[[nodiscard]] static pal::result<parsed_message<msturn>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;
};

} // namespace turner
