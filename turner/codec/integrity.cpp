#include <turner/codec/integrity.hpp>
#include <pal/crypto/hash.hpp>
#include <pal/crypto/hmac.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <utility>
#include <variant>

namespace turner::__integrity
{

pal::result<hmac> hmac::sha1 (std::span<const std::byte> key) noexcept
{
	return sha1_type::make(key).transform([](sha1_type &&keyed) noexcept
	{
		return hmac{impl_type{std::move(keyed)}};
	});
}

pal::result<hmac> hmac::sha256 (std::span<const std::byte> key) noexcept
{
	return sha256_type::make(key).transform([](sha256_type &&keyed) noexcept
	{
		return hmac{impl_type{std::move(keyed)}};
	});
}

integrity_algorithm hmac::algorithm () const noexcept
{
	return std::holds_alternative<sha1_type>(impl_) ? integrity_algorithm::sha1 : integrity_algorithm::sha256;
}

size_t hmac::digest_size () const noexcept
{
	return std::holds_alternative<sha1_type>(impl_) ? sha1_type::digest_size : sha256_type::digest_size;
}

void hmac::update (std::span<const std::byte> input) noexcept
{
	if (auto *sha1 = std::get_if<sha1_type>(&impl_))
	{
		sha1->update(input);
	}
	else
	{
		std::get_if<sha256_type>(&impl_)->update(input);
	}
}

void hmac::finish (std::span<std::byte> digest) noexcept
{
	if (auto *sha1 = std::get_if<sha1_type>(&impl_))
	{
		std::ranges::copy(sha1->finish(), digest.begin());
	}
	else
	{
		std::ranges::copy(std::get_if<sha256_type>(&impl_)->finish(), digest.begin());
	}
}

bool hmac::finish_matches (std::span<const std::byte> expected) noexcept
{
	std::array<std::byte, sha256_type::digest_size> digest{};
	finish(std::span{digest}.first(expected.size()));

	// timing must not reveal the first mismatching byte
	volatile std::byte difference{};
	for (size_t i = 0; i < expected.size(); ++i)
	{
		difference = difference | (digest[i] ^ expected[i]);
	}
	return difference == std::byte{};
}

pal::result<std::array<std::byte, 16>> md5_key (const integrity_credentials &credentials) noexcept
{
	using namespace std::string_view_literals;
	return pal::crypto::md5_hash::one_shot(
		credentials.username.bytes(),
		":"sv,
		credentials.realm.bytes(),
		":"sv,
		credentials.password.bytes()
	);
}

} // namespace turner::__integrity
