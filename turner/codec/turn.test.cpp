#include <turner/codec/turn.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/stun.hpp>
#include <concepts>

namespace
{

using turner::msturn;
using turner::stun;
using turner::turn;

template <auto M>
concept has_indication = requires
{
	M.indication;
};

template <auto M>
concept has_success = requires
{
	M.success;
};

template <auto M>
concept has_error = requires
{
	M.error;
};

static_assert(turn::allocate.type == 0x0003);
static_assert(turn::allocate.success.type == 0x0103);
static_assert(turn::allocate.error.type == 0x0113);
static_assert(turn::refresh.type == 0x0004);
static_assert(turn::refresh.success.type == 0x0104);
static_assert(turn::refresh.error.type == 0x0114);
static_assert(turn::send_indication.type == 0x0016);
static_assert(turn::data_indication.type == 0x0017);
static_assert(turn::create_permission.type == 0x0008);
static_assert(turn::create_permission.success.type == 0x0108);
static_assert(turn::create_permission.error.type == 0x0118);
static_assert(turn::channel_bind.type == 0x0009);
static_assert(turn::channel_bind.success.type == 0x0109);
static_assert(turn::channel_bind.error.type == 0x0119);
static_assert(turn::binding.request.type == 0x0001);
static_assert(std::same_as<decltype(turn::binding), decltype(stun::binding)>);
static_assert(turn::header_size == stun::header_size);

static_assert(!has_indication<turn::allocate>);
static_assert(!has_success<turn::send_indication> && !has_error<turn::send_indication>);
static_assert(!has_success<turn::data_indication> && !has_error<turn::data_indication>);

static_assert(turner::message<decltype(turn::allocate)>);
static_assert(turner::message<decltype(turn::send_indication)>);
static_assert(!turner::message<decltype(turn::binding)>);

static_assert(turner::usable_with<decltype(turn::allocate), turn>);
static_assert(turner::usable_with<decltype(turn::binding), turn>);
static_assert(turner::usable_with<decltype(turn::binding.request), turn>);
static_assert(!turner::usable_with<decltype(turn::allocate), stun>);
static_assert(!turner::usable_with<decltype(msturn::allocate), turn>);

} // namespace
