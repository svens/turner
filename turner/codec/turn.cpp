#include <turner/codec/turn.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <pal/result.hpp>
#include <cstddef>
#include <span>

namespace turner
{

pal::result<parsed_message<turn>> turn::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	const auto parsed = stun::parse(buffer, received);
	if (!parsed)
	{
		return pal::unexpected{parsed.error()};
	}
	return parsed_message<turn>{buffer};
}

} // namespace turner
