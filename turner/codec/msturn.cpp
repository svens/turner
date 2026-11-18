#include <turner/codec/msturn.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/writer.hpp>
#include <pal/crypto/hmac.hpp>
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

namespace
{

struct header_and_cookie
{
	pal::big_endian<uint16_t> type;
	pal::big_endian<uint16_t> length;
	msturn::transaction_id transaction_id;
	std::array<std::byte, 8> cookie;
};

static_assert(pal::packed_struct<header_and_cookie>);
static_assert(sizeof(header_and_cookie) == msturn::header_size + msturn::cookie.size());
static_assert(offsetof(header_and_cookie, transaction_id) == msturn::transaction_id_offset);
static_assert(offsetof(header_and_cookie, cookie) == msturn::cookie_offset);
static_assert(sizeof(header_and_cookie::cookie) == msturn::cookie.size());

constexpr struct
{
	uint16_t type = 0x0008;
	size_t sha1_size = pal::crypto::sha1_hmac::digest_size;
	size_t sha256_size = pal::crypto::sha256_hmac::digest_size;
	size_t hmac_input_alignment = 64;
} message_integrity;

using __wire::attribute_header_size;

constexpr uint16_t mturn_channel_id = 0xff10;
constexpr size_t mturn_session_id_size = msturn::mturn_session_id::extent;
constexpr unsigned frame_kind_shift = 8;

bool starts_with_message (std::span<const std::byte> bytes) noexcept
{
	if (bytes.size() < sizeof(header_and_cookie))
	{
		return false;
	}
	const auto &h = pal::packed_as<header_and_cookie>(bytes.first<sizeof(header_and_cookie)>());
	return (h.type & __wire::type_reserved_bits) == 0 && h.cookie == msturn::cookie;
}

bool is_mturn_packet (std::span<const std::byte> bytes) noexcept
{
	if (bytes.size() < msturn::mturn_header_size)
	{
		return false;
	}
	const auto &header = __wire::type_and_length::read(bytes);
	return header.type == mturn_channel_id && attribute_header_size + header.length == bytes.size();
}

void update_padded (__integrity::hmac &key, std::span<const std::byte> covered) noexcept
{
	static constexpr std::array<std::byte, message_integrity.hmac_input_alignment> zeros{};
	key.update(covered);
	key.update(
		std::span{zeros}.first(
			__wire::align_up(covered.size(), message_integrity.hmac_input_alignment) - covered.size()
		)
	);
}

} // namespace

integrity_algorithm msturn::integrity_type::algorithm (
	std::span<const std::byte> message,
	size_t attributes_end) noexcept
{
	const auto attribute = msturn::header_size + attributes_end;
	if (attribute == message.size())
	{
		return integrity_algorithm::none;
	}
	const auto &header = __wire::type_and_length::read(message.subspan(attribute));
	return header.length == message_integrity.sha1_size ? integrity_algorithm::sha1 : integrity_algorithm::sha256;
}

pal::result<void> msturn::integrity_type::verify (
	__integrity::hmac &key,
	std::span<const std::byte> message,
	size_t attributes_end) noexcept
{
	if (algorithm(message, attributes_end) != key.algorithm())
	{
		return make_unexpected(errc::integrity_missing);
	}
	const auto attribute = msturn::header_size + attributes_end;
	update_padded(key, message.first(attribute));
	if (!key.finish_matches(message.subspan(attribute + attribute_header_size)))
	{
		return make_unexpected(errc::integrity_mismatch);
	}
	return {};
}

void msturn::integrity_type::write (
	__integrity::hmac &key,
	std::span<std::byte> message,
	size_t attribute_offset) noexcept
{
	__wire::type_and_length::write(message.subspan(attribute_offset), message_integrity.type, key.digest_size());
	update_padded(key, message.first(attribute_offset));
	key.finish(message.subspan(attribute_offset + attribute_header_size));
}

pal::result<msturn::integrity_key> msturn::sha1_key (const integrity_credentials &credentials) noexcept
{
	return __integrity::md5_key(credentials).and_then([](const auto &key) noexcept
	{
		return integrity_key::sha1(key);
	});
}

pal::result<msturn::integrity_key> msturn::sha256_key (const integrity_credentials &credentials) noexcept
{
	using hmac = pal::crypto::sha256_hmac;
	static constexpr std::array prefix
	{
		std::byte{0x01}, std::byte{'T'}, std::byte{'U'}, std::byte{'R'}, std::byte{'N'}, std::byte{0x00},
	};
	static constexpr std::array suffix{std::byte{0x00}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00}};

	return hmac::one_shot(credentials.nonce.bytes(), credentials.password.bytes())
		.and_then([&](const auto &initial_key) noexcept
		{
			return hmac::one_shot(
				initial_key,
				prefix,
				credentials.username.bytes(),
				credentials.realm.bytes(),
				suffix
			);
		})
		.and_then([](const auto &key) noexcept
		{
			return integrity_key::sha256(key);
		});
}

content msturn::peek (std::span<const std::byte> datagram) noexcept
{
	if (starts_with_message(datagram))
	{
		return content::message;
	}
	if (is_mturn_packet(datagram))
	{
		return content::data;
	}
	return content::other;
}

pal::result<msturn::frame> msturn::parse_frame (std::span<const std::byte> buffer, size_t received) noexcept
{
	if (received < frame_header_size)
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &header = __wire::type_and_length::read(buffer);
	const auto kind = static_cast<frame_kind>(header.type >> frame_kind_shift);
	if (kind != frame_kind::control && kind != frame_kind::data)
	{
		return make_unexpected(errc::unexpected_message_type);
	}

	const size_t length = header.length;
	if (frame_header_size + length > received)
	{
		return make_unexpected(errc::incomplete);
	}
	return frame{kind, buffer.subspan(frame_header_size, length)};
}

std::span<const std::byte> msturn::make_frame (
	std::span<std::byte> buffer,
	std::span<const std::byte> payload,
	frame_kind kind) noexcept
{
	const auto result = buffer.subspan(
		__wire::offset_of(payload, buffer) - frame_header_size,
		frame_header_size + payload.size()
	);
	__wire::type_and_length::write(
		result,
		static_cast<uint16_t>(std::to_underlying(kind) << frame_kind_shift),
		payload.size()
	);
	return result;
}

pal::result<msturn::mturn_data> msturn::parse_mturn_data (std::span<const std::byte> buffer, size_t received) noexcept
{
	if (received < mturn_header_size)
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &header = __wire::type_and_length::read(buffer);
	if (header.type != mturn_channel_id)
	{
		return make_unexpected(errc::unexpected_message_type);
	}

	const size_t length = header.length;
	if (length < mturn_session_id_size)
	{
		return make_unexpected(errc::unexpected_message_length);
	}
	if (attribute_header_size + length > received)
	{
		return make_unexpected(errc::incomplete);
	}
	return mturn_data{
		buffer.subspan<attribute_header_size, mturn_session_id_size>(),
		buffer.subspan(mturn_header_size, length - mturn_session_id_size),
	};
}

std::span<const std::byte> msturn::make_mturn_data (
	std::span<std::byte> buffer,
	std::span<const std::byte> payload,
	mturn_session_id session_id) noexcept
{
	std::array<std::byte, mturn_session_id_size> id{};
	std::ranges::copy(session_id, id.begin());

	const auto packet = buffer.subspan(
		__wire::offset_of(payload, buffer) - mturn_header_size,
		mturn_header_size + payload.size()
	);
	__wire::type_and_length::write(packet, mturn_channel_id, mturn_session_id_size + payload.size());
	std::ranges::copy(id, packet.subspan<attribute_header_size, mturn_session_id_size>().begin());
	return packet;
}

std::span<const std::byte> msturn::make_data_indication (
	std::span<std::byte> buffer,
	std::span<const std::byte> payload,
	const transport_address &peer,
	const transaction_id &id) noexcept
{
	using peer_attribute = decltype(remote_address);
	using peer_value = peer_attribute::value_type;

	constexpr auto attributes_offset = __wire::attributes_offset<msturn>;
	const auto peer_size = peer_value::size(peer);
	const auto prefix_size = attributes_offset + attribute_header_size + peer_size + attribute_header_size;
	const auto message = buffer.subspan(
		__wire::offset_of(payload, buffer) - prefix_size,
		prefix_size + payload.size()
	);

	__writer::write_header<msturn>(message, data_indication.type, id);
	__wire::type_and_length::write(message, data_indication.type, message.size() - header_size);

	const auto attributes = message.subspan(attributes_offset);
	__wire::type_and_length::write(attributes, peer_attribute::type, peer_size);
	peer_value::write(message, attributes.subspan(attribute_header_size, peer_size), peer);

	const auto data_attribute = attributes.subspan(attribute_header_size + peer_size);
	__wire::type_and_length::write(data_attribute, decltype(data)::type, payload.size());
	return message;
}

pal::result<parsed_message<msturn>> msturn::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	if (received < sizeof(header_and_cookie))
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &h = pal::packed_as<header_and_cookie>(buffer.first<sizeof(header_and_cookie)>());
	if ((h.type & __wire::type_reserved_bits) != 0)
	{
		return make_unexpected(errc::unexpected_message_type);
	}
	if (h.cookie != cookie)
	{
		return make_unexpected(errc::invalid_magic_cookie);
	}

	const uint16_t length = h.length;
	if (length < cookie.size())
	{
		return make_unexpected(errc::unexpected_message_length);
	}
	if (header_size + length > received)
	{
		return make_unexpected(errc::incomplete);
	}

	uint16_t attributes_end = length;

	auto attributes = buffer.subspan(sizeof(header_and_cookie), length - cookie.size());
	while (!attributes.empty())
	{
		if (attributes.size() < attribute_header_size)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}
		const auto &attribute = __wire::type_and_length::read(attributes);
		const size_t attribute_length = attribute.length;
		if (attribute_length > attributes.size() - attribute_header_size)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}
		const auto next = attributes.subspan(attribute_header_size + attribute_length);
		if (attribute.type == message_integrity.type)
		{
			if (!next.empty())
			{
				return make_unexpected(errc::integrity_not_last);
			}
			if (attribute_length != message_integrity.sha1_size
				&& attribute_length != message_integrity.sha256_size)
			{
				return make_unexpected(errc::unexpected_attribute_length);
			}
			attributes_end = static_cast<uint16_t>(length - attributes.size());
		}
		attributes = next;
	}

	return parsed_message<msturn>{buffer, attributes_end};
}

} // namespace turner
