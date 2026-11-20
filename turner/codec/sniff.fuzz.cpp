#include <turner/codec/fuzz.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/sniff.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <pal/fuzz.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <system_error>

namespace
{

using turner::session_protocol;
using turner::transport;
using turner_fuzz::require;

constexpr size_t max_input =
	turner::msturn::frame_header_size
	+ turner::msturn::header_size
	+ turner::msturn::cookie.size();

bool passes_cookie_checks (std::error_code error)
{
	return !turner_fuzz::failed_with(error, turner::errc::unexpected_message_type)
		&& !turner_fuzz::failed_with(error, turner::errc::invalid_magic_cookie);
}

template <typename Protocol>
bool parse_passes_cookie_checks (std::span<const std::byte> bytes)
{
	auto buffer = turner_fuzz::copy_of(bytes);
	const auto parsed = Protocol::parse(buffer, buffer.size());
	return parsed || passes_cookie_checks(parsed.error());
}

void check_datagram (std::span<const std::byte> input)
{
	const auto protocol = turner::sniff(input, transport::datagram);
	require(protocol != session_protocol::need_more);
	require((protocol == session_protocol::msturn) == (turner::msturn::peek(input) == turner::content::message));
	if (protocol == session_protocol::stun)
	{
		require(parse_passes_cookie_checks<turner::turn>(input));
	}
	if (protocol == session_protocol::msturn)
	{
		require(parse_passes_cookie_checks<turner::msturn>(input));
	}
}

void check_stream (std::span<const std::byte> input)
{
	auto decided = session_protocol::need_more;
	for (size_t size = 0; size <= input.size(); ++size)
	{
		const auto prefix = turner_fuzz::copy_of(input.first(size));
		const auto protocol = turner::sniff(prefix, transport::stream);
		if (decided != session_protocol::need_more)
		{
			require(protocol == decided);
		}
		decided = protocol;
	}

	if (decided == session_protocol::stun && input.size() >= turner::stun::header_size)
	{
		require(parse_passes_cookie_checks<turner::turn>(input));
	}
	if (decided == session_protocol::msturn)
	{
		if (const auto frame = turner::msturn::parse_frame(input, input.size()))
		{
			require(frame->kind() == turner::msturn::frame_kind::control);
		}
	}
}

} // namespace

extern "C" int LLVMFuzzerInitialize (int *argc, char ***argv)
{
	pal::fuzz::init(argc, argv, max_input);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, std::size_t size)
{
	const std::span input{reinterpret_cast<const std::byte *>(data), size};
	check_datagram(input);
	check_stream(input);
	return 0;
}
