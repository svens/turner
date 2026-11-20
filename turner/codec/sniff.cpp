#include <turner/codec/sniff.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/stun.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

namespace turner
{

namespace
{

constexpr uint8_t last_stream_stun_first_byte = 1;
constexpr uint8_t msturn_control_frame_first_byte = std::to_underlying(msturn::frame_kind::control);
constexpr size_t stun_cookie_end = stun::cookie_offset + stun::cookie.size();
constexpr size_t msturn_cookie_offset = msturn::frame_header_size + msturn::cookie_offset;
constexpr size_t msturn_cookie_end = msturn_cookie_offset + msturn::cookie.size();
constexpr size_t msturn_smallest_message = msturn::header_size + msturn::cookie.size();

uint8_t first_byte (std::span<const std::byte> bytes) noexcept
{
	return std::to_integer<uint8_t>(bytes.front());
}

session_protocol sniff_datagram (std::span<const std::byte> bytes) noexcept
{
	if (msturn::peek(bytes) == content::message)
	{
		return session_protocol::msturn;
	}
	if (bytes.size() >= stun::header_size
		&& first_byte(bytes) <= __wire::stun_first_byte_max
		&& __wire::matches_at(bytes, stun::cookie_offset, stun::cookie))
	{
		return session_protocol::stun;
	}
	return session_protocol::other;
}

session_protocol sniff_stun_stream (std::span<const std::byte> bytes) noexcept
{
	if (!__wire::matches_at(bytes, stun::cookie_offset, stun::cookie))
	{
		return session_protocol::other;
	}
	return bytes.size() < stun_cookie_end ? session_protocol::need_more : session_protocol::stun;
}

session_protocol sniff_msturn_stream (std::span<const std::byte> bytes) noexcept
{
	if (bytes.size() >= msturn::frame_header_size
		&& __wire::type_and_length::read(bytes).length < msturn_smallest_message)
	{
		return session_protocol::other;
	}
	if (bytes.size() > msturn::frame_header_size
		&& ((first_byte(bytes.subspan(msturn::frame_header_size)) << 8) & __wire::type_reserved_bits) != 0)
	{
		return session_protocol::other;
	}
	if (!__wire::matches_at(bytes, msturn_cookie_offset, msturn::cookie))
	{
		return session_protocol::other;
	}
	return bytes.size() < msturn_cookie_end ? session_protocol::need_more : session_protocol::msturn;
}

session_protocol sniff_stream (std::span<const std::byte> bytes) noexcept
{
	if (bytes.empty())
	{
		return session_protocol::need_more;
	}

	const auto first = first_byte(bytes);
	if (first <= last_stream_stun_first_byte)
	{
		return sniff_stun_stream(bytes);
	}
	if (first == msturn_control_frame_first_byte)
	{
		return sniff_msturn_stream(bytes);
	}
	return session_protocol::other;
}

} // namespace

session_protocol sniff (std::span<const std::byte> bytes, transport transport) noexcept
{
	return transport == transport::datagram ? sniff_datagram(bytes) : sniff_stream(bytes);
}

} // namespace turner
