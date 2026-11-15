#include <turner/codec/msturn.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <array>
#include <cstddef>

namespace
{

using turner::msturn;
using turner::stun;
using turner::turn;

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

static_assert(sizeof(msturn::transaction_id) == 16);
static_assert(msturn::transaction_id_offset == 4);
static_assert(msturn::header_size == 20);
static_assert(msturn::cookie_offset == 20);
static_assert(
	msturn::cookie == std::array
	{
		std::byte{0x00}, std::byte{0x0f}, std::byte{0x00}, std::byte{0x04},
		std::byte{0x72}, std::byte{0xc6}, std::byte{0x4b}, std::byte{0xc6},
	}
);
static_assert(msturn::header_size == msturn::transaction_id_offset + sizeof(msturn::transaction_id));
static_assert(msturn::attribute_alignment == 1);

static_assert(msturn::allocate.type == 0x0003);
static_assert(msturn::allocate.success.type == 0x0103);
static_assert(msturn::allocate.error.type == 0x0113);
static_assert(msturn::send_request.type == 0x0004);
static_assert(msturn::data_indication.type == 0x0115);
static_assert(msturn::data_indication.indication.type == 0x0115);
static_assert(turner::message_of<decltype(msturn::data_indication)>::type == 0x0115);
static_assert(turner::indication<turn, 0x005>.type == 0x0015);
static_assert(msturn::set_active_destination.type == 0x0006);
static_assert(msturn::set_active_destination.success.type == 0x0106);
static_assert(msturn::set_active_destination.error.type == 0x0116);

static_assert(!has_success<msturn::send_request> && !has_error<msturn::send_request>);
static_assert(!has_success<msturn::data_indication> && !has_error<msturn::data_indication>);

static_assert(turner::message<decltype(msturn::send_request)>);
static_assert(turner::message<decltype(msturn::data_indication)>);

static_assert(turner::usable_with<decltype(msturn::allocate), msturn>);
static_assert(!turner::usable_with<decltype(turn::allocate), msturn>);
static_assert(!turner::usable_with<decltype(stun::binding), msturn>);

} // namespace
