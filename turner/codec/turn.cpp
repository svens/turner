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
	const auto attributes_end = __stun::parse(buffer, received);
	if (!attributes_end)
	{
		return pal::unexpected{attributes_end.error()};
	}
	return parsed_message<turn>{buffer, *attributes_end};
}

} // namespace turner
