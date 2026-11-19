#include <turner/codec/fuzz.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <pal/crypto/hash.hpp>
#include <pal/fuzz.hpp>
#include <pal/result.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <string_view>
#include <utility>

namespace
{

using turner::stun;
using turner_fuzz::require;

constexpr size_t max_input = stun::header_size + 0xffff;

struct keys_type
{
	stun::integrity_key sha1;
	stun::integrity_key sha256;
	stun::integrity_key md5_key_sha256;
	stun::integrity_key empty_realm;
	stun::integrity_key rfc5769_short_term;
	stun::integrity_key rfc5769_long_term;
};

const turner::integrity_credentials credentials
{
	.username = "user",
	.realm = "realm",
	.nonce = "",
	.password = "pass",
};

stun::integrity_key valid (pal::result<stun::integrity_key> &&key)
{
	require(key.has_value());
	return std::move(*key);
}

pal::crypto::md5_hash::digest_type md5_key ()
{
	const auto digest = pal::crypto::md5_hash::one_shot(std::string_view{"user:realm:pass"});
	require(digest.has_value());
	return *digest;
}

keys_type &keys ()
{
	static keys_type keys
	{
		.sha1 = valid(stun::sha1_key(credentials)),
		.sha256 = valid(stun::sha256_key(credentials)),
		.md5_key_sha256 = valid(stun::integrity_key::sha256(md5_key())),
		.empty_realm = valid(stun::sha1_key({
			.username = "user",
			.realm = "",
			.nonce = "",
			.password = "pass",
		})),
		.rfc5769_short_term = valid(stun::integrity_key::sha1("VOkJxbRl1RmTxUk/WvJxBt")),
		.rfc5769_long_term = valid(stun::sha1_key({
			.username = "\xe3\x83\x9e\xe3\x83\x88\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9",
			.realm = "example.org",
			.nonce = "",
			.password = "TheMatrIX",
		})),
	};
	return keys;
}

template <auto Message>
void read_message (
	const turner::parsed_message<stun> &parsed,
	std::span<const std::byte> message)
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
		stun::alternate_server
	>(*reader, message);
	auto &k = keys();
	turner_fuzz::check_integrity(
		*reader,
		k.sha1,
		k.sha256,
		k.md5_key_sha256,
		k.empty_realm,
		k.rfc5769_short_term,
		k.rfc5769_long_term
	);
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

	turner_fuzz::byte_buffer buffer;
	const auto message = turner_fuzz::parse_twice<stun>(input, buffer);
	if (message.empty())
	{
		return 0;
	}

	const auto parsed = stun::parse(buffer, size);
	require(parsed.has_value());
	read_message<stun::binding.request>(*parsed, message);
	read_message<stun::binding.indication>(*parsed, message);
	read_message<stun::binding.success>(*parsed, message);
	read_message<stun::binding.error>(*parsed, message);
	require(turner_fuzz::same_bytes(buffer, input));

	turner_fuzz::check_transitions<stun, stun::binding.request, stun::binding.success, stun::binding.error>(
		message,
		keys().sha1,
		turner::integrity_algorithm::sha1
	);
	turner_fuzz::check_transitions<stun, stun::binding.request, stun::binding.success, stun::binding.error>(
		message,
		keys().sha256,
		turner::integrity_algorithm::sha256
	);
	return 0;
}
