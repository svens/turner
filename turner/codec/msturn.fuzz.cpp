#include <turner/codec/fuzz.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/reader.hpp>
#include <pal/fuzz.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <system_error>
#include <utility>

namespace
{

using turner::msturn;
using turner_fuzz::inside;
using turner_fuzz::require;

constexpr size_t max_input = msturn::header_size + 0xffff;

struct keys_type
{
	msturn::integrity_key sha1;
	msturn::integrity_key sha256;
	msturn::integrity_key sha256_long_nonce;
};

const turner::integrity_credentials credentials
{
	.username = "user",
	.realm = "realm",
	.nonce = "nonce",
	.password = "pass",
};

msturn::integrity_key valid (pal::result<msturn::integrity_key> &&key)
{
	require(key.has_value());
	return std::move(*key);
}

keys_type &keys ()
{
	static keys_type keys
	{
		.sha1 = valid(msturn::sha1_key(credentials)),
		.sha256 = valid(msturn::sha256_key(credentials)),
		.sha256_long_nonce = valid(msturn::sha256_key({
			.username = "user",
			.realm = "realm",
			.nonce = std::string(65, 'n'),
			.password = "pass",
		})),
	};
	return keys;
}

template <auto Message>
void read_msturn_message (const turner::parsed_message<msturn> &parsed, std::span<const std::byte> message)
{
	const auto reader = parsed.as(Message);
	if (!reader)
	{
		return;
	}
	turner_fuzz::read_attributes<
		msturn::mapped_address,
		msturn::username,
		msturn::error_code,
		msturn::unknown_attributes,
		msturn::lifetime,
		msturn::alternate_server,
		msturn::bandwidth,
		msturn::destination_address,
		msturn::remote_address,
		msturn::data,
		msturn::nonce,
		msturn::realm,
		msturn::requested_address_family,
		msturn::ms_version,
		msturn::xor_mapped_address,
		msturn::ms_alternate_host_name,
		msturn::app_id,
		msturn::secure_tag,
		msturn::ms_sequence_number,
		msturn::ms_service_quality,
		msturn::ms_alternate_mapped_address,
		msturn::multiplexed_session_id
	>(*reader, message);
	turner_fuzz::check_integrity(*reader, keys().sha1, keys().sha256, keys().sha256_long_nonce);
}

template <auto... Message>
void read_msturn_messages (const turner::parsed_message<msturn> &parsed, std::span<const std::byte> message)
{
	(read_msturn_message<Message>(parsed, message), ...);
}

template <auto Method>
void check_method_transitions (std::span<const std::byte> message)
{
	turner_fuzz::check_transitions<msturn, Method.request, Method.success, Method.error>(
		message,
		keys().sha1,
		turner::integrity_algorithm::sha1
	);
	turner_fuzz::check_transitions<msturn, Method.request, Method.success, Method.error>(
		message,
		keys().sha256,
		turner::integrity_algorithm::sha256
	);
}

void check_messages (std::span<const std::byte> input)
{
	turner_fuzz::byte_buffer buffer;
	const auto message = turner_fuzz::parse_twice<msturn>(input, buffer);
	if (message.empty())
	{
		return;
	}

	const auto parsed = msturn::parse(buffer, input.size());
	require(parsed.has_value());
	read_msturn_messages<
		msturn::allocate.request,
		msturn::allocate.success,
		msturn::allocate.error,
		msturn::send_request,
		msturn::data_indication,
		msturn::set_active_destination.request,
		msturn::set_active_destination.success,
		msturn::set_active_destination.error
	>(*parsed, message);
	require(turner_fuzz::same_bytes(buffer, input));

	check_method_transitions<msturn::allocate>(message);
	check_method_transitions<msturn::set_active_destination>(message);
}

void check_peek (std::span<const std::byte> input)
{
	const auto content = msturn::peek(input);
	if (content == turner::content::message)
	{
		auto buffer = turner_fuzz::copy_of(input);
		const auto parsed = msturn::parse(buffer, buffer.size());
		require(parsed || (
			!turner_fuzz::failed_with(parsed.error(), turner::errc::unexpected_message_type)
			&& !turner_fuzz::failed_with(parsed.error(), turner::errc::invalid_magic_cookie)
		));
	}
	else if (content == turner::content::data)
	{
		const auto packet = msturn::parse_mturn_data(input, input.size());
		require(packet.has_value() && packet->size() == input.size());
	}
	require(content != turner::content::need_more);
}

void check_frames (std::span<const std::byte> input)
{
	const auto buffer = turner_fuzz::copy_of(input);

	const auto tailed = turner_fuzz::copy_of(input, 16);
	{
		const auto exact = msturn::parse_frame(buffer, buffer.size());
		const auto with_tail = msturn::parse_frame(tailed, input.size());
		require(exact.has_value() == with_tail.has_value());
		require(!exact || (exact->size() == with_tail->size() && exact->kind() == with_tail->kind()));
		require(exact || exact.error() == with_tail.error());
	}
	{
		const auto exact = msturn::parse_mturn_data(buffer, buffer.size());
		const auto with_tail = msturn::parse_mturn_data(tailed, input.size());
		require(exact.has_value() == with_tail.has_value());
		require(!exact || exact->size() == with_tail->size());
		require(exact || exact.error() == with_tail.error());
	}

	if (const auto frame = msturn::parse_frame(buffer, buffer.size()))
	{
		require(frame->size() <= buffer.size());
		require(inside(frame->payload(), std::span{buffer}.first(frame->size())));
		turner_fuzz::touch(frame->payload());
	}

	if (const auto packet = msturn::parse_mturn_data(buffer, buffer.size()))
	{
		require(packet->size() <= buffer.size());
		const auto first = std::span{buffer}.first(packet->size());
		require(inside(packet->payload(), first));
		require(inside(packet->session_id(), first));
		turner_fuzz::touch(packet->payload());
		turner_fuzz::touch(packet->session_id());
	}

	std::optional<std::error_code> decided;
	const auto check_prefix = [&](size_t size)
	{
		const auto prefix = turner_fuzz::copy_of(input.first(size));
		const auto result = msturn::parse_pseudo_tls(prefix);
		if (decided)
		{
			require(!result ? result.error() == *decided : *decided == std::error_code{});
		}
		else if (result)
		{
			decided = std::error_code{};
		}
		else if (!turner_fuzz::failed_with(result.error(), turner::errc::incomplete))
		{
			decided = result.error();
		}
	};

	const auto last = (std::min)(input.size(), msturn::pseudo_tls_client_hello_size + 1);
	for (size_t size = 0; size <= last; ++size)
	{
		check_prefix(size);
	}
	if (input.size() > last)
	{
		check_prefix(input.size());
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
	check_peek(input);
	check_frames(input);
	return 0;
}
