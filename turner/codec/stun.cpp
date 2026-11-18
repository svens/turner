#include <turner/codec/stun.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/reader.hpp>
#include <pal/crypto/hash.hpp>
#include <pal/crypto/hmac.hpp>
#include <pal/hash.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace turner
{

namespace
{

struct header
{
	pal::big_endian<uint16_t> type;
	pal::big_endian<uint16_t> length;
	std::array<std::byte, 4> cookie;
	stun::transaction_id transaction_id;
};

static_assert(pal::packed_struct<header>);
static_assert(sizeof(header) == stun::header_size);
static_assert(offsetof(header, cookie) == stun::cookie_offset);
static_assert(sizeof(header::cookie) == stun::cookie.size());
static_assert(offsetof(header, transaction_id) == stun::transaction_id_offset);

using __wire::attribute_header_size;

struct integrity_attribute
{
	uint16_t type;
	size_t value_size;
	size_t size = attribute_header_size + value_size;
};

constexpr integrity_attribute message_integrity
{
	.type = 0x0008,
	.value_size = pal::crypto::sha1_hmac::digest_size,
};

constexpr integrity_attribute message_integrity_sha256
{
	.type = 0x001c,
	.value_size = pal::crypto::sha256_hmac::digest_size,
};

constexpr uint16_t fingerprint_attribute_type = 0x8028;
constexpr uint32_t fingerprint_xor = 0x5354'554e;

uint32_t fingerprint_of (std::span<const std::byte> covered) noexcept
{
	return pal::crc32(covered) ^ fingerprint_xor;
}

const __wire::type_and_length &attribute_at (std::span<const std::byte> message, size_t offset) noexcept
{
	return __wire::type_and_length::read(message.subspan(stun::header_size + offset));
}

std::optional<size_t> message_integrity_offset (std::span<const std::byte> message, size_t attributes_end) noexcept
{
	if (attributes_end + stun::header_size < message.size()
		&& attribute_at(message, attributes_end).type == message_integrity.type)
	{
		return attributes_end;
	}
	return std::nullopt;
}

std::optional<size_t> message_integrity_sha256_offset (
	std::span<const std::byte> message,
	size_t attributes_end) noexcept
{
	if (attributes_end + stun::header_size == message.size())
	{
		return std::nullopt;
	}
	if (attribute_at(message, attributes_end).type == message_integrity_sha256.type)
	{
		return attributes_end;
	}
	if (message_integrity_offset(message, attributes_end))
	{
		const auto next = attributes_end + message_integrity.size;
		if (next + stun::header_size < message.size()
			&& attribute_at(message, next).type == message_integrity_sha256.type)
		{
			return next;
		}
	}
	return std::nullopt;
}

} // namespace

integrity_algorithm stun::integrity_type::algorithm (
	std::span<const std::byte> message,
	size_t attributes_end) noexcept
{
	if (message_integrity_sha256_offset(message, attributes_end))
	{
		return integrity_algorithm::sha256;
	}
	if (message_integrity_offset(message, attributes_end))
	{
		return integrity_algorithm::sha1;
	}
	return integrity_algorithm::none;
}

pal::result<void> stun::integrity_type::verify (
	__integrity::hmac &key,
	std::span<const std::byte> message,
	size_t attributes_end) noexcept
{
	const auto offset = key.algorithm() == integrity_algorithm::sha1
		? message_integrity_offset(message, attributes_end)
		: message_integrity_sha256_offset(message, attributes_end);
	if (!offset)
	{
		return make_unexpected(errc::integrity_missing);
	}

	const auto &received = __wire::type_and_length::read(message);
	const __wire::type_and_length patched
	{
		.type = received.type,
		.length = static_cast<uint16_t>(*offset + attribute_header_size + key.digest_size()),
	};
	const auto attribute = stun::header_size + *offset;
	key.update(std::as_bytes(std::span{&patched, 1}));
	key.update(message.first(attribute).subspan(attribute_header_size));

	if (!key.finish_matches(message.subspan(attribute + attribute_header_size, key.digest_size())))
	{
		return make_unexpected(errc::integrity_mismatch);
	}
	return {};
}

void stun::integrity_type::write (
	__integrity::hmac &key,
	std::span<std::byte> message,
	size_t attribute_offset) noexcept
{
	__wire::type_and_length::write(
		message.subspan(attribute_offset),
		key.algorithm() == integrity_algorithm::sha1 ? message_integrity.type : message_integrity_sha256.type,
		key.digest_size()
	);
	key.update(message.first(attribute_offset));
	key.finish(message.subspan(attribute_offset + attribute_header_size));
}

void stun::fingerprint_type::write (std::span<std::byte, size> attribute, std::span<const std::byte> covered) noexcept
{
	static_assert(size == attribute_header_size + sizeof(uint32_t));
	__wire::type_and_length::write(attribute, fingerprint_attribute_type, sizeof(uint32_t));
	pal::packed_as<pal::big_endian<uint32_t>>(attribute.last<sizeof(uint32_t)>()) = fingerprint_of(covered);
}

pal::result<stun::integrity_key> stun::sha1_key (const integrity_credentials &credentials) noexcept
{
	return __integrity::md5_key(credentials).and_then([](const auto &key) noexcept
	{
		return integrity_key::sha1(key);
	});
}

pal::result<stun::integrity_key> stun::sha256_key (const integrity_credentials &credentials) noexcept
{
	using namespace std::string_view_literals;
	return pal::crypto::sha256_hash::one_shot(
		credentials.username.bytes(),
		":"sv,
		credentials.realm.bytes(),
		":"sv,
		credentials.password.bytes()
	).and_then([](const auto &key) noexcept
	{
		return integrity_key::sha256(key);
	});
}

pal::result<parsed_message<stun>> stun::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	const auto attributes_end = __stun::parse(buffer, received);
	if (!attributes_end)
	{
		return pal::unexpected{attributes_end.error()};
	}
	return parsed_message<stun>{buffer, *attributes_end};
}

pal::result<uint16_t> __stun::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	constexpr auto header_size = stun::header_size;
	constexpr auto attribute_alignment = stun::attribute_alignment;

	if (received < header_size)
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &h = pal::packed_as<header>(buffer.first<header_size>());
	if ((h.type & __wire::type_reserved_bits) != 0)
	{
		return make_unexpected(errc::unexpected_message_type);
	}
	if (h.cookie != stun::cookie)
	{
		return make_unexpected(errc::invalid_magic_cookie);
	}

	const uint16_t length = h.length;
	if (length % attribute_alignment != 0)
	{
		return make_unexpected(errc::unexpected_message_length);
	}
	if (header_size + length > received)
	{
		return make_unexpected(errc::incomplete);
	}

	uint16_t attributes_end = length;
	size_t second_integrity_offset = length;

	auto attributes = buffer.subspan(header_size, length);
	while (!attributes.empty())
	{
		const auto offset = length - attributes.size();
		const auto &attribute = __wire::type_and_length::read(attributes);
		const auto padded_length = __wire::align_up(attribute.length, attribute_alignment);
		if (padded_length > attributes.size() - attribute_header_size)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}
		const auto next = attributes.subspan(attribute_header_size + padded_length);

		const uint16_t type = attribute.type;
		if (attributes_end == length
			&& (type == message_integrity.type
				|| type == message_integrity_sha256.type
				|| type == fingerprint_attribute_type))
		{
			attributes_end = static_cast<uint16_t>(offset);
			if (type == message_integrity.type)
			{
				second_integrity_offset = offset + message_integrity.size;
			}
		}
		if ((offset == attributes_end
				&& type == message_integrity.type
				&& attribute.length != message_integrity.value_size)
			|| ((offset == attributes_end || offset == second_integrity_offset)
				&& type == message_integrity_sha256.type
				&& attribute.length != message_integrity_sha256.value_size))
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}

		if (type == fingerprint_attribute_type)
		{
			if (!next.empty())
			{
				return make_unexpected(errc::fingerprint_not_last);
			}
			if (attribute.length != sizeof(uint32_t))
			{
				return make_unexpected(errc::unexpected_attribute_length);
			}
			const auto &value = pal::packed_as<pal::big_endian<uint32_t>>(
				attributes.subspan<attribute_header_size, sizeof(uint32_t)>()
			);
			const auto covered = buffer.first(header_size + length - attributes.size());
			if (value != fingerprint_of(covered))
			{
				return make_unexpected(errc::fingerprint_mismatch);
			}
		}
		attributes = next;
	}

	return attributes_end;
}

} // namespace turner
