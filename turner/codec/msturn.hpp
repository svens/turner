#pragma once

/**
 * \file turner/codec/msturn.hpp
 * MS-TURN protocol
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
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

template <uint8_t... Byte>
inline constexpr std::array<std::byte, sizeof...(Byte)> bytes{std::byte{Byte}...};

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

	/// Wire format of Message Integrity, used by message reader and writer
	class integrity_type
	{
		[[nodiscard]] static integrity_algorithm algorithm (
			std::span<const std::byte> message,
			size_t attributes_end
		) noexcept;

		[[nodiscard]] static pal::result<void> verify (
			__integrity::hmac &key,
			std::span<const std::byte> message,
			size_t attributes_end
		) noexcept;

		static void write (
			__integrity::hmac &key,
			std::span<std::byte> message,
			size_t attribute_offset
		) noexcept;

		template <typename>
		friend class basic_message_reader;

		template <typename>
		friend class basic_message_writer;
	};

	/// Key of Message Integrity
	using integrity_key = turner::integrity_key<msturn>;

	/// Create HMAC-SHA1 key from \a credentials: MD5(username ":" realm ":" password). Nonce is not used.
	///
	/// \ref realm value is read with its quotes, while credentials realm is expected without them: MS-TURN says
	/// "value of the Realm attribute", and the draft it defers to for this key removes the quotes.
	/// \see https://learn.microsoft.com/openspecs/office_protocols/ms-turn/d9dd90bc-5bd3-4daa-89ac-0d70c5a1cb81
	[[nodiscard]] static pal::result<integrity_key> sha1_key (const integrity_credentials &credentials) noexcept;

	/// Create HMAC-SHA256 key from \a credentials: K = HMAC-SHA256(nonce, password),
	/// Key = HMAC-SHA256(K, 0x01 "TURN" 0x00 username realm 0x00000100). Key depends on nonce: create new one when
	/// nonce changes.
	///
	/// Credentials realm is expected without quotes: MS-TURN says "value of the Realm attribute" here as well, and
	/// deployed servers remove the quotes.
	[[nodiscard]] static pal::result<integrity_key> sha256_key (const integrity_credentials &credentials) noexcept;

	/// Validate message in first \a received bytes of \a buffer. \a buffer is whole I/O buffer: response to the
	/// message is built in it. \a received must not exceed \a buffer size.
	///
	/// Checks message structure only (header, Magic Cookie, attribute layout, Message Integrity length and
	/// position), not whether message type or attributes are known. Fails with errc::incomplete if \a received
	/// bytes do not hold whole message. Bytes following the message are ignored: use parsed_message::size() to
	/// find them.
	[[nodiscard]] static pal::result<parsed_message<msturn>> parse (
		std::span<std::byte> buffer,
		size_t received
	) noexcept;

	/// Size of pseudo-TLS ClientHello
	static constexpr size_t pseudo_tls_client_hello_size = 50;

	/// Pseudo-TLS record with ServerHello and ServerHelloDone, the reply to pseudo-TLS ClientHello. It is not
	/// framed. Time and random value are constant, while specification says server should send current time and
	/// generated random value.
	static constexpr auto pseudo_tls_server_hello = __msturn::bytes<
		0x16, 0x03, 0x01, 0x00, 0x4e, 0x02, 0x00, 0x00,
		0x46, 0x03, 0x01, 0x94, 0x86, 0xbb, 0x57, 0x42,
		0xb0, 0x78, 0xd8, 0xc6, 0x2a, 0x0b, 0x59, 0x0b,
		0x20, 0xad, 0x42, 0xe7, 0x5a, 0xba, 0x9b, 0x5d,
		0xa7, 0x79, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x20, 0x42, 0xb0, 0x78, 0xd8,
		0xc6, 0x2a, 0x0b, 0x59, 0x0b, 0x20, 0xad, 0x42,
		0xe7, 0x5a, 0xba, 0x9b, 0x5d, 0xa7, 0x79, 0xfd,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x0e,
		0x00, 0x00, 0x00
	>;

	/// Check whether \a bytes received first on TCP connection start with pseudo-TLS ClientHello. Fails with
	/// errc::unexpected_message_type as soon as a field with value required by specification differs, and with
	/// errc::incomplete if the bytes match so far but do not hold whole ClientHello. Time and random value are not
	/// checked, and bytes following the ClientHello are ignored. After it, only framed MS-TURN may follow.
	[[nodiscard]] static pal::result<void> parse_pseudo_tls (std::span<const std::byte> bytes) noexcept;

	/// Size of TCP framing header
	static constexpr size_t frame_header_size = 4;

	/// Most bytes make_frame() and make_data_indication() together write before payload
	static constexpr size_t max_headroom =
		frame_header_size
		+ header_size + cookie.size()
		+ __wire::attribute_header_size + decltype(remote_address)::value_type::max_size
		+ __wire::attribute_header_size;

	/// Most bytes written after payload
	static constexpr size_t max_tailroom = 0;

	/// Largest payload of make_data_indication() whose message fits make_frame() for every peer address family
	static constexpr size_t max_payload = 0xffff - (max_headroom - frame_header_size);

	/// Return what \a datagram is: message with Magic Cookie, Multiplexed TURN packet or other, which is raw data
	/// of any content. \a datagram must be whole, so content::need_more is not returned and empty \a datagram is
	/// content::other. On TCP, use parse_frame() instead.
	[[nodiscard]] static content peek (std::span<const std::byte> datagram) noexcept;

	/// Type of data in TCP frame
	enum class frame_kind: uint8_t
	{
		control = 2, ///< TURN control message
		data = 3, ///< End-to-end data
	};

	/// Validated TCP frame. Created by parse_frame().
	class frame
	{
	public:

		/// Return type of data in frame
		[[nodiscard]] frame_kind kind () const noexcept
		{
			return kind_;
		}

		/// Return data following framing header. It refers to bytes of buffer given to parse_frame().
		[[nodiscard]] std::span<const std::byte> payload () const noexcept
		{
			return payload_;
		}

		/// Return number of received bytes the frame occupies
		[[nodiscard]] size_t size () const noexcept
		{
			return frame_header_size + payload_.size();
		}

	private:

		std::span<const std::byte> payload_;
		frame_kind kind_;

		frame (frame_kind kind, std::span<const std::byte> payload) noexcept
			: payload_{payload}
			, kind_{kind}
		{
		}

		friend msturn;
	};

	/// Validate TCP frame in first \a received bytes of \a buffer. \a received must not exceed \a buffer size.
	/// Fails with errc::unexpected_message_type if frame type is not known, and with errc::incomplete if
	/// \a received bytes do not hold whole frame. Reserved byte is ignored. Bytes following the frame are ignored:
	/// use frame::size() to find them. Parse control message with parse() over buffer following framing header,
	/// passing payload size of the frame as received bytes.
	[[nodiscard]] static pal::result<frame> parse_frame (
		std::span<const std::byte> buffer,
		size_t received
	) noexcept;

	/// Make TCP frame of \a kind around \a payload in \a buffer and return it. \a payload must refer to bytes of
	/// \a buffer, also when empty, be at most 0xffff bytes and have \ref frame_header_size bytes before it in
	/// \a buffer. These are not checked.
	[[nodiscard]] static std::span<const std::byte> make_frame (
		std::span<std::byte> buffer,
		std::span<const std::byte> payload,
		frame_kind kind
	) noexcept;

	/// Multiplexed TURN session ID: same type as value of \ref multiplexed_session_id. It refers to bytes it does
	/// not own.
	using mturn_session_id = std::span<const std::byte, 8>;

	/// Size of Multiplexed TURN packet header: Channel_ID, length and session ID
	static constexpr size_t mturn_header_size = __wire::attribute_header_size + mturn_session_id::extent;

	/// Validated Multiplexed TURN packet. Created by parse_mturn_data().
	class mturn_data
	{
	public:

		/// Return session ID. It refers to bytes of buffer given to parse_mturn_data().
		[[nodiscard]] mturn_session_id session_id () const noexcept
		{
			return session_id_;
		}

		/// Return wrapped data. It refers to bytes of buffer given to parse_mturn_data().
		[[nodiscard]] std::span<const std::byte> payload () const noexcept
		{
			return payload_;
		}

		/// Return number of received bytes the packet occupies
		[[nodiscard]] size_t size () const noexcept
		{
			return mturn_header_size + payload_.size();
		}

	private:

		mturn_session_id session_id_;
		std::span<const std::byte> payload_;

		mturn_data (mturn_session_id session_id, std::span<const std::byte> payload) noexcept
			: session_id_{session_id}
			, payload_{payload}
		{
		}

		friend msturn;
	};

	/// Validate Multiplexed TURN packet in first \a received bytes of \a buffer. \a received must not exceed
	/// \a buffer size. Fails with errc::incomplete if \a received bytes do not hold whole packet, with
	/// errc::unexpected_message_type if it does not start with its Channel_ID, and with
	/// errc::unexpected_message_length if its length does not cover session ID. Bytes following the packet are
	/// ignored: use mturn_data::size() to find them.
	[[nodiscard]] static pal::result<mturn_data> parse_mturn_data (
		std::span<const std::byte> buffer,
		size_t received
	) noexcept;

	/// Make Multiplexed TURN packet of \a session_id around \a payload in \a buffer and return it. \a payload must
	/// refer to bytes of \a buffer, also when empty, be at most 0xffff - 8 bytes and have \ref mturn_header_size
	/// bytes before it in \a buffer. These are not checked. \a session_id may refer to those bytes.
	[[nodiscard]] static std::span<const std::byte> make_mturn_data (
		std::span<std::byte> buffer,
		std::span<const std::byte> payload,
		mturn_session_id session_id
	) noexcept;

	/// Make Data Indication from \a peer with \a id around \a payload in \a buffer and return it. \a payload must
	/// refer to bytes of \a buffer, also when empty, be at most \ref max_payload bytes and have
	/// \ref max_headroom - \ref frame_header_size bytes before it in \a buffer. \a peer and \a id must not refer to
	/// bytes of \a buffer. These are not checked.
	[[nodiscard]] static std::span<const std::byte> make_data_indication (
		std::span<std::byte> buffer,
		std::span<const std::byte> payload,
		const transport_address &peer,
		const transaction_id &id
	) noexcept;
};

} // namespace turner
