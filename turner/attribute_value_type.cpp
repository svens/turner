#include <turner/attribute_value_type>

#if __has_include(<arpa/inet.h>)
	#include <arpa/inet.h>
#elif __has_include(<ws2tcpip.h>)
	#include <ws2tcpip.h>
#endif

namespace turner::__attribute_value_type {

void xor_op::transform (
	const std::span<const std::byte> &message_bytes,
	pal::net::ip::address &address,
	uint16_t &port) noexcept
{
	// - MS-TURN: 16B Transaction ID
	// - TURN: 4B Magic Cookie + 12B Transaction ID
	auto &xor_value = *reinterpret_cast<const std::array<uint8_t, 16> *>(
		message_bytes.data() + 4
	);

	port ^= *reinterpret_cast<const uint16_t *>(xor_value.data());
	if (address.is_v4())
	{
		auto &in = *const_cast<in_addr *>(
			reinterpret_cast<const in_addr *>(address.v4().to_bytes().data())
		);
		in.s_addr ^= *reinterpret_cast<const uint32_t *>(xor_value.data());
	}
	else
	{
		auto &in = *const_cast<in6_addr *>(
			reinterpret_cast<const in6_addr *>(address.v6().to_bytes().data())
		);
		for (auto i = 0u;  i < xor_value.max_size();  ++i)
		{
			in.s6_addr[i] ^= xor_value[i];
		}
	}
}

} // namespace turner::__attribute_value_type
