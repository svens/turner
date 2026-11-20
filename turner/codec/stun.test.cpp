#include <turner/codec/stun.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <array>
#include <cstddef>

namespace
{

using turner::stun;

static_assert(sizeof(stun::transaction_id) == 12);
static_assert(stun::transaction_id_offset == 8);
static_assert(stun::header_size == 20);
static_assert(stun::cookie_offset == 4);
static_assert(stun::cookie == std::array{std::byte{0x21}, std::byte{0x12}, std::byte{0xa4}, std::byte{0x42}});
static_assert(stun::attribute_alignment == 4);
static_assert(stun::transaction_id_offset == stun::cookie_offset + stun::cookie.size());
static_assert(stun::header_size == stun::transaction_id_offset + sizeof(stun::transaction_id));

static_assert(stun::binding.request.type == 0x0001);
static_assert(stun::binding.indication.type == 0x0011);
static_assert(stun::binding.success.type == 0x0101);
static_assert(stun::binding.error.type == 0x0111);

static_assert(!turner::message<decltype(stun::binding)>);
static_assert(turner::message<decltype(stun::binding.request)>);
static_assert(turner::message<decltype(stun::binding.indication)>);

static_assert(turner::usable_with<decltype(stun::binding), stun>);

} // namespace
