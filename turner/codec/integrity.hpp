#pragma once

/**
 * \file turner/codec/integrity.hpp
 * Message integrity keys
 */

#include <pal/crypto/hmac.hpp>
#include <pal/result.hpp>
#include <array>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <variant>

namespace turner
{

template <typename Message>
class basic_message_reader;

template <typename Message>
class basic_message_writer;

/// Message integrity algorithm
enum class integrity_algorithm
{
	none, ///< Message has no integrity attribute
	sha1, ///< HMAC-SHA1
	sha256, ///< HMAC-SHA256
};

namespace __integrity
{

class octets
{
public:

	template <typename Text>
		requires std::convertible_to<const Text &, std::string_view>
	octets (const Text &text) noexcept
		: bytes_{std::as_bytes(std::span{std::string_view{text}})}
	{
	}

	template <std::ranges::contiguous_range Bytes>
		requires std::ranges::sized_range<Bytes>
			&& std::same_as<std::ranges::range_value_t<Bytes>, std::byte>
	octets (const Bytes &bytes) noexcept
		: bytes_{bytes}
	{
	}

	[[nodiscard]] std::span<const std::byte> bytes () const noexcept
	{
		return bytes_;
	}

private:

	std::span<const std::byte> bytes_;
};

} // namespace __integrity

/// Credentials that integrity keys are derived from. Each field is text or bytes, refers to caller's storage that
/// must stay valid until key is derived, and is hashed as given: preprocessing is up to caller. Every field is
/// required; protocols and algorithms that do not use \ref nonce ignore it, so pass empty value there.
struct integrity_credentials
{
	__integrity::octets username; ///< User name
	__integrity::octets realm; ///< Realm
	__integrity::octets nonce; ///< Nonce, used only by msturn::sha256_key()
	__integrity::octets password; ///< Password
};

namespace __integrity
{

class hmac
{
public:

	[[nodiscard]] static pal::result<hmac> sha1 (std::span<const std::byte> key) noexcept;
	[[nodiscard]] static pal::result<hmac> sha256 (std::span<const std::byte> key) noexcept;

	[[nodiscard]] integrity_algorithm algorithm () const noexcept;
	[[nodiscard]] size_t digest_size () const noexcept;

	void update (std::span<const std::byte> input) noexcept;
	void finish (std::span<std::byte> digest) noexcept;
	[[nodiscard]] bool finish_matches (std::span<const std::byte> expected) noexcept;

private:

	using sha1_type = pal::crypto::sha1_hmac;
	using sha256_type = pal::crypto::sha256_hmac;
	using impl_type = std::variant<sha1_type, sha256_type>;

	impl_type impl_;

	explicit hmac (impl_type &&impl) noexcept
		: impl_{std::move(impl)}
	{
	}
};

[[nodiscard]] pal::result<std::array<std::byte, 16>> md5_key (const integrity_credentials &credentials) noexcept;

} // namespace __integrity

/// Key of message integrity attribute, ready for basic_message_reader::verify() and
/// basic_message_writer::finish(). It is keyed once on creation, so these calls do not allocate; they update its
/// state and need exclusive access to it. Application keeps it in its session or authentication context.
///
/// Key for \ref stun is usable also with \ref turn messages.
template <typename Protocol>
class integrity_key
{
public:

	/// Protocol that defines the key
	using protocol_type = Protocol;

	/// Create HMAC-SHA1 key from key bytes \a key, given as text or bytes
	[[nodiscard]] static pal::result<integrity_key> sha1 (__integrity::octets key) noexcept
	{
		return make(__integrity::hmac::sha1(key.bytes()));
	}

	/// Create HMAC-SHA256 key from key bytes \a key, given as text or bytes
	[[nodiscard]] static pal::result<integrity_key> sha256 (__integrity::octets key) noexcept
	{
		return make(__integrity::hmac::sha256(key.bytes()));
	}

private:

	__integrity::hmac hmac_;

	explicit integrity_key (__integrity::hmac &&hmac) noexcept
		: hmac_{std::move(hmac)}
	{
	}

	[[nodiscard]] static pal::result<integrity_key> make (pal::result<__integrity::hmac> &&hmac) noexcept
	{
		return std::move(hmac).transform([](__integrity::hmac &&keyed) noexcept
		{
			return integrity_key{std::move(keyed)};
		});
	}

	template <typename>
	friend class basic_message_reader;

	template <typename>
	friend class basic_message_writer;
};

} // namespace turner
