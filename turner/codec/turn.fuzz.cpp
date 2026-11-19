#include <turner/codec/fuzz.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <pal/fuzz.hpp>
#include <pal/result.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <utility>

namespace
{

using turner::stun;
using turner::turn;
using turner_fuzz::inside;
using turner_fuzz::require;

constexpr size_t max_input = turn::header_size + 0xffff;

struct keys_type
{
	turn::integrity_key sha1;
	turn::integrity_key sha256;
};

const turner::integrity_credentials credentials
{
	.username = "user",
	.realm = "realm",
	.nonce = "",
	.password = "pass",
};

turn::integrity_key valid (pal::result<turn::integrity_key> &&key)
{
	require(key.has_value());
	return std::move(*key);
}

keys_type &keys ()
{
	static keys_type keys
	{
		.sha1 = valid(turn::sha1_key(credentials)),
		.sha256 = valid(turn::sha256_key(credentials)),
	};
	return keys;
}

template <auto Message>
void read_turn_message (const turner::parsed_message<turn> &parsed, std::span<const std::byte> message)
{
	const auto reader = parsed.as(Message);
	if (!reader)
	{
		return;
	}
	turner_fuzz::read_attributes<
		stun::mapped_address,
		stun::username,
		stun::error_code,
		stun::unknown_attributes,
		stun::realm,
		stun::nonce,
		stun::password_algorithm,
		stun::userhash,
		stun::xor_mapped_address,
		stun::password_algorithms,
		stun::alternate_domain,
		stun::software,
		stun::alternate_server,
		turn::channel_number,
		turn::lifetime,
		turn::xor_peer_address,
		turn::data,
		turn::xor_relayed_address,
		turn::requested_address_family,
		turn::requested_transport,
		turn::dont_fragment,
		turn::additional_address_family,
		turn::address_error_code
	>(*reader, message);
	turner_fuzz::check_integrity(*reader, keys().sha1, keys().sha256);
}

template <auto... Message>
void read_turn_messages (const turner::parsed_message<turn> &parsed, std::span<const std::byte> message)
{
	(read_turn_message<Message>(parsed, message), ...);
}

template <auto Method>
void check_method_transitions (std::span<const std::byte> message)
{
	turner_fuzz::check_transitions<turn, Method.request, Method.success, Method.error>(
		message,
		keys().sha1,
		turner::integrity_algorithm::sha1
	);
	turner_fuzz::check_transitions<turn, Method.request, Method.success, Method.error>(
		message,
		keys().sha256,
		turner::integrity_algorithm::sha256
	);
}

void check_messages (std::span<const std::byte> input)
{
	turner_fuzz::byte_buffer buffer;
	const auto message = turner_fuzz::parse_twice<turn>(input, buffer);
	if (message.empty())
	{
		return;
	}

	const auto parsed = turn::parse(buffer, input.size());
	require(parsed.has_value());
	read_turn_messages<
		turn::allocate.request,
		turn::allocate.success,
		turn::allocate.error,
		turn::refresh.request,
		turn::refresh.success,
		turn::refresh.error,
		turn::create_permission.request,
		turn::create_permission.success,
		turn::create_permission.error,
		turn::channel_bind.request,
		turn::channel_bind.success,
		turn::channel_bind.error,
		turn::send_indication,
		turn::data_indication
	>(*parsed, message);
	if (const auto binding = parsed->as(turn::binding.request))
	{
		turner_fuzz::read_attributes<
			stun::username,
			stun::software,
			stun::xor_mapped_address
		>(*binding, message);
	}
	require(turner_fuzz::same_bytes(buffer, input));

	check_method_transitions<turn::allocate>(message);
	check_method_transitions<turn::refresh>(message);
	check_method_transitions<turn::create_permission>(message);
	check_method_transitions<turn::channel_bind>(message);
	check_method_transitions<turn::binding>(message);
}

void check_channel_data (std::span<const std::byte> input)
{
	const auto buffer = turner_fuzz::copy_of(input);
	const auto datagram = turn::parse_channel_data(buffer, buffer.size(), turner::transport::datagram);
	const auto stream = turn::parse_channel_data(buffer, buffer.size(), turner::transport::stream);

	const auto tailed = turner_fuzz::copy_of(input, 16);
	for (const auto transport: {turner::transport::datagram, turner::transport::stream})
	{
		const auto exact = turn::parse_channel_data(buffer, buffer.size(), transport);
		const auto with_tail = turn::parse_channel_data(tailed, input.size(), transport);
		require(exact.has_value() == with_tail.has_value());
		if (exact)
		{
			require(exact->channel() == with_tail->channel());
			require(exact->size() == with_tail->size());
			require(exact->payload().size() == with_tail->payload().size());
		}
		else
		{
			require(exact.error() == with_tail.error());
		}
	}

	for (const auto *parsed: {&datagram, &stream})
	{
		if (*parsed)
		{
			const auto &frame = **parsed;
			require(frame.size() <= buffer.size());
			require(inside(frame.payload(), std::span{buffer}.first(frame.size())));
			turner_fuzz::touch(frame.payload());
		}
	}
	if (stream)
	{
		require(datagram.has_value());
		require(datagram->channel() == stream->channel());
		require(datagram->payload().data() == stream->payload().data());
		require(datagram->payload().size() == stream->payload().size());
	}

	if (input.size() >= 4)
	{
		const bool is_data = turn::peek(input) == turner::content::data;
		const bool channel_ok = datagram || !turner_fuzz::failed_with(
			datagram.error(),
			turner::errc::unexpected_message_type
		);
		require(is_data == channel_ok);
	}
}

} // namespace

extern "C" int LLVMFuzzerInitialize (int *argc, char ***argv)
{
	pal::fuzz::init(argc, argv, max_input);
	std::ignore = keys();
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, std::size_t size)
{
	const std::span input{reinterpret_cast<const std::byte *>(data), size};
	check_messages(input);
	check_channel_data(input);
	return 0;
}
