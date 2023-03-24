#include <turner/stun>
#include <turner/__view>
#include <turner/error>
#include <pal/byte_order>

namespace turner {

namespace {

using message_view = __view::message<stun>;
using attribute_view = __view::attribute<stun>;

template <uint32_t Polynomial>
struct crc32_lookup
{
	uint32_t table[4][256];

	constexpr crc32_lookup () noexcept
	{
		for (uint32_t i = 0;  i <= 0xff;  ++i)
		{
			uint32_t crc = i;
			for (int j = 0;  j < 8;  ++j)
			{
				crc = (crc >> 1) ^ ((crc & 1) * Polynomial);
			}
			table[0][i] = crc;
		}

		for (uint32_t i = 0;  i <= 0xff;  ++i)
		{
			table[1][i] = (table[0][i] >> 8) ^ table[0][(uint8_t)table[0][i]];
			table[2][i] = (table[1][i] >> 8) ^ table[0][(uint8_t)table[1][i]];
			table[3][i] = (table[2][i] >> 8) ^ table[0][(uint8_t)table[2][i]];
		}
	}
};

uint32_t crc32 (const uint32_t *first, const uint32_t *last) noexcept
{
	// Intel 32bit CRC using Slice-by-4 method
	// See ThirdPartySources.txt for copyright notices

	static constexpr crc32_lookup<0xedb88320> lookup{};

	uint32_t crc = ~0U;
	while (first != last)
	{
		crc ^= *first++;
		crc =
			lookup.table[3][(uint8_t)(crc      )] ^
			lookup.table[2][(uint8_t)(crc >>  8)] ^
			lookup.table[1][(uint8_t)(crc >> 16)] ^
			lookup.table[0][(uint8_t)(crc >> 24)]
		;
	}

	// STUN messages are padded to 4B boundary, can skip final 0..3

	return ~crc;
}

} // namespace

pal::result<stun::message_reader> stun::read_message (const std::span<const std::byte> &span) noexcept
{
	constexpr auto min_span_size_bytes = header_size_bytes;

	auto span_size_bytes = span.size_bytes();
	if (span_size_bytes < min_span_size_bytes || span_size_bytes % pad_size_bytes != 0)
	{
		return make_unexpected(errc::unexpected_message_length);
	}

	auto &message = *reinterpret_cast<const message_view *>(span.data());
	if (message.cookie() != magic_cookie)
	{
		return make_unexpected(errc::invalid_magic_cookie);
	}

	if (message.payload_size_bytes() + header_size_bytes != span_size_bytes)
	{
		return make_unexpected(errc::unexpected_message_length);
	}

	if (message.type() & 0b1100'0000'0000'0000)
	{
		return make_unexpected(errc::unexpected_message_type);
	}

	// Iterate attributes:
	// - validate lengths
	// - check optional fingerprint attribute is last
	// - if present, check it matches expected fingerprint

	auto it = message.begin(), end = message.end();
	while (it != end)
	{
		auto &attr = *it;
		it = it->next();

		if (it > end)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}

		if (attr.type() == fingerprint.type)
		{
			if (it != end)
			{
				return make_unexpected(errc::fingerprint_not_last);
			}

			auto value = attr.value();
			if (value.size_bytes() != sizeof(uint32_t))
			{
				return make_unexpected(errc::unexpected_attribute_length);
			}

			auto claimed_crc = pal::ntoh(*reinterpret_cast<const uint32_t *>(value.data()));
			auto expected_crc = 0x5354554e ^ crc32(
				reinterpret_cast<const uint32_t *>(span.data()),
				reinterpret_cast<const uint32_t *>(&attr)
			);

			if (expected_crc != claimed_crc)
			{
				return make_unexpected(errc::fingerprint_mismatch);
			}
		}
	}

	return message_reader{span};
}

} // namespace turner
