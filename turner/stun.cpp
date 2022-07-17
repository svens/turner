#include <turner/stun>
#include <turner/error>
#include <pal/byte_order>


__turner_begin


namespace {

constexpr size_t header_size = 20, pad_size = 4;

using cookie_type = std::array<uint8_t, 4>;
constexpr cookie_type magic_cookie = { 0x21, 0x12, 0xa4, 0x42 };

struct header_ptr
{
	uint16_t request () const noexcept
	{
		return pal::ntoh(reinterpret_cast<const uint16_t *>(this)[0]);
	}

	const cookie_type &cookie () const noexcept
	{
		return reinterpret_cast<const cookie_type *>(this)[1];
	}

	size_t length () const noexcept
	{
		return pal::ntoh(reinterpret_cast<const uint16_t *>(this)[1]);
	}
};

struct attribute_ptr
{
	const std::byte *ptr () const noexcept
	{
		return reinterpret_cast<const std::byte *>(this);
	}

	uint16_t type () const noexcept
	{
		return pal::ntoh(reinterpret_cast<const uint16_t *>(this)[0]);
	}

	uint16_t value_size () const noexcept
	{
		return pal::ntoh(reinterpret_cast<const uint16_t *>(this)[1]);
	}

	size_t size () const noexcept
	{
		return 2 * sizeof(uint16_t) + ((value_size() + pad_size - 1) & ~(pad_size - 1));
	}
};

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

	// STUN messages are padded to 4B bounday, can skip final 0..3

	return ~crc;
}

} // namespace


pal::result<stun::message_reader> stun::read_message (std::span<const std::byte> span) noexcept
{
	auto ptr = span.data();
	auto size = span.size_bytes();

	if (size < header_size || size % 4 != 0)
	{
		return make_unexpected(errc::unexpected_message_length);
	}

	auto &header = *reinterpret_cast<const header_ptr *>(ptr);
	if (header.cookie() != magic_cookie)
	{
		return make_unexpected(errc::invalid_magic_cookie);
	}

	if (header.length() + header_size != size)
	{
		return make_unexpected(errc::unexpected_message_length);
	}

	if (header.request() & 0b1100'0000'0000'0000)
	{
		return make_unexpected(errc::unexpected_message_type);
	}

	const auto end = ptr + size;
	ptr += header_size;

	// Iterate attributes:
	// - validate lengths
	// - check optional fingerprint attribute is last
	// - if present, check it matches expected fingerprint
	while (ptr != end)
	{
		auto &attr = *reinterpret_cast<const attribute_ptr *>(ptr);

		ptr += attr.size();
		if (ptr > end)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}

		if (attr.type() == fingerprint)
		{
			if (ptr != end)
			{
				return make_unexpected(errc::fingerprint_not_last);
			}

			auto claimed_crc = pal::ntoh(reinterpret_cast<const uint32_t *>(&attr)[1]);
			auto expected_crc = crc32(
				reinterpret_cast<const uint32_t *>(span.data()),
				reinterpret_cast<const uint32_t *>(attr.ptr())
			);

			if ((expected_crc ^ 0x5354554e) != claimed_crc)
			{
				return make_unexpected(errc::fingerprint_mismatch);
			}
		}
	}

	return message_reader{span};
}


__turner_end
