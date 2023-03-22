#include <turner/msturn>
#include <turner/__view>
#include <turner/error>
#include <pal/byte_order>

namespace turner {

namespace {

using message_view = __view::message<msturn>;
using attribute_view = __view::attribute<msturn>;

} // namespace

pal::result<msturn::message_reader> msturn::read_message (const std::span<const std::byte> &span) noexcept
{
	constexpr auto min_span_size_bytes = header_size_bytes + magic_cookie.size();

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

	auto it = message.begin(), end = message.end();
	while (it < end)
	{
		it = it->next();
	}

	if (it == end)
	{
		return message_reader{span};
	}

	return make_unexpected(errc::unexpected_attribute_length);
}

} // namespace turner
