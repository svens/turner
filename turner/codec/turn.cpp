#include <turner/codec/turn.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/writer.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

namespace turner
{

namespace
{

using __wire::attribute_header_size;

constexpr uint8_t last_message_first_byte = 3;
constexpr uint8_t first_channel_first_byte = channel_number_value::first >> 8;
constexpr uint8_t last_channel_first_byte = channel_number_value::last >> 8;

void zero_padding (std::span<std::byte> padding) noexcept
{
	std::ranges::fill(padding, std::byte{});
}

} // namespace

pal::result<parsed_message<turn>> turn::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	const auto attributes_end = __stun::parse(buffer, received);
	if (!attributes_end)
	{
		return pal::unexpected{attributes_end.error()};
	}
	return parsed_message<turn>{buffer, *attributes_end};
}

content turn::peek (std::span<const std::byte> bytes) noexcept
{
	if (bytes.empty())
	{
		return content::need_more;
	}

	const auto first = std::to_integer<uint8_t>(bytes.front());
	if (first <= last_message_first_byte)
	{
		return content::message;
	}
	if (first_channel_first_byte <= first && first <= last_channel_first_byte)
	{
		return content::data;
	}
	return content::other;
}

pal::result<turn::channel_data> turn::parse_channel_data (
	std::span<const std::byte> buffer,
	size_t received,
	turner::transport transport) noexcept
{
	if (received < attribute_header_size)
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &header = __wire::type_and_length::read(buffer);
	const uint16_t channel = header.type;
	if (channel < channel_number_value::first || channel > channel_number_value::last)
	{
		return make_unexpected(errc::unexpected_message_type);
	}

	const size_t length = header.length;
	const auto unpadded_size = attribute_header_size + length;
	const auto padded_size = __wire::align_up(unpadded_size, attribute_alignment);
	const auto required_size = transport == turner::transport::stream ? padded_size : unpadded_size;
	if (required_size > received)
	{
		return make_unexpected(errc::incomplete);
	}

	return channel_data{
		channel,
		buffer.subspan(attribute_header_size, length),
		(std::min)(padded_size, received),
	};
}

std::span<const std::byte> turn::make_channel_data (
	std::span<std::byte> buffer,
	std::span<const std::byte> payload,
	uint16_t channel) noexcept
{
	const auto unpadded_size = attribute_header_size + payload.size();
	const auto frame = buffer.subspan(
		__wire::offset_of(payload, buffer) - attribute_header_size,
		__wire::align_up(unpadded_size, attribute_alignment)
	);
	__wire::type_and_length::write(frame, channel, payload.size());
	zero_padding(frame.subspan(unpadded_size));
	return frame;
}

std::span<const std::byte> turn::make_data_indication (
	std::span<std::byte> buffer,
	std::span<const std::byte> payload,
	const transport_address &peer,
	const transaction_id &id) noexcept
{
	using peer_attribute = decltype(xor_peer_address);
	using peer_value = peer_attribute::value_type;

	const auto peer_size = peer_value::size(peer);
	const auto prefix_size = attribute_header_size + peer_size + attribute_header_size;
	const auto length = prefix_size + __wire::align_up(payload.size(), attribute_alignment);
	const auto frame = buffer.subspan(
		__wire::offset_of(payload, buffer) - header_size - prefix_size,
		header_size + length
	);

	__writer::write_header<turn>(frame, data_indication.type, id);
	__wire::type_and_length::write(frame, data_indication.type, length);

	const auto attributes = frame.subspan(header_size);
	__wire::type_and_length::write(attributes, peer_attribute::type, peer_size);
	peer_value::write(frame, attributes.subspan(attribute_header_size, peer_size), peer);

	const auto data_attribute = attributes.subspan(attribute_header_size + peer_size);
	__wire::type_and_length::write(data_attribute, decltype(data)::type, payload.size());
	zero_padding(data_attribute.subspan(attribute_header_size + payload.size()));
	return frame;
}

} // namespace turner
