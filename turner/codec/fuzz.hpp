#pragma once

/**
 * \file turner/codec/fuzz.hpp
 * Fuzz target helpers
 * \internal
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/protocol_error.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace turner_fuzz
{

inline constexpr size_t read_check_limit = 4096;

inline void require (bool condition)
{
	if (!condition)
	{
		std::abort();
	}
}

inline void touch (std::span<const std::byte> bytes)
{
	volatile std::byte sink{};
	for (const auto byte: bytes)
	{
		sink = sink ^ byte;
	}
}

inline bool same_bytes (std::span<const std::byte> a, std::span<const std::byte> b)
{
	return a.size() == b.size() && (a.empty() || std::memcmp(a.data(), b.data(), a.size()) == 0);
}

inline bool inside (std::span<const std::byte> part, std::span<const std::byte> whole)
{
	return std::less_equal<>{}(whole.data(), part.data())
		&& std::less_equal<>{}(part.data() + part.size(), whole.data() + whole.size());
}

inline void check_view (std::span<const std::byte> view, std::span<const std::byte> message)
{
	require(inside(view, message));
	touch(view);
}

template <typename T>
void check_value (const T &value, std::span<const std::byte> message)
{
	if constexpr (std::convertible_to<const T &, std::span<const std::byte>>)
	{
		check_view(value, message);
	}
	else if constexpr (std::same_as<T, std::string_view>)
	{
		check_view(std::as_bytes(std::span{value}), message);
	}
	else if constexpr (requires { value.code; value.reason; })
	{
		check_view(std::as_bytes(std::span{value.reason}), message);
	}
	else if constexpr (requires { value.family; value.error; })
	{
		check_value(value.error, message);
	}
}

template <typename T>
bool same_value (const T &a, const T &b)
{
	if constexpr (requires { a.code; a.reason; })
	{
		return a.code == b.code && a.reason == b.reason;
	}
	else if constexpr (requires { a.family; a.error; })
	{
		return a.family == b.family && same_value(a.error, b.error);
	}
	else if constexpr (requires { { a == b } -> std::convertible_to<bool>; })
	{
		return a == b;
	}
	else
	{
		static_assert(std::ranges::range<T>, "native type needs comparison here");
		return std::ranges::equal(a, b);
	}
}

template <typename T>
bool same (const pal::result<T> &a, const pal::result<T> &b)
{
	if (a.has_value() != b.has_value())
	{
		return false;
	}
	if (!a)
	{
		return a.error() == b.error();
	}
	return same_value(*a, *b);
}

class byte_buffer
{
public:

	byte_buffer () = default;

	explicit byte_buffer (std::span<const std::byte> bytes, size_t tailroom = 0)
		: data_{std::make_unique_for_overwrite<std::byte[]>(bytes.size() + tailroom)}
		, size_{bytes.size() + tailroom}
	{
		if (!bytes.empty())
		{
			std::memcpy(data_.get(), bytes.data(), bytes.size());
		}
		if (tailroom != 0)
		{
			std::memset(data_.get() + bytes.size(), 0xa5, tailroom);
		}
	}

	byte_buffer (const byte_buffer &that)
		: byte_buffer{std::span<const std::byte>{that}}
	{
	}

	byte_buffer (byte_buffer &&) noexcept = default;
	byte_buffer &operator= (byte_buffer &&) noexcept = default;
	byte_buffer &operator= (const byte_buffer &) = delete;
	~byte_buffer () = default;

	[[nodiscard]] std::byte *data () const noexcept
	{
		return data_.get();
	}

	[[nodiscard]] size_t size () const noexcept
	{
		return size_;
	}

	[[nodiscard]] std::byte *begin () const noexcept
	{
		return data_.get();
	}

	[[nodiscard]] std::byte *end () const noexcept
	{
		return data_.get() + size_;
	}

	friend bool operator== (const byte_buffer &a, const byte_buffer &b) noexcept
	{
		return a.size_ == b.size_ && (a.size_ == 0 || std::memcmp(a.data(), b.data(), a.size_) == 0);
	}

private:

	std::unique_ptr<std::byte[]> data_;
	size_t size_ = 0;
};

inline byte_buffer copy_of (std::span<const std::byte> bytes, size_t tailroom = 0)
{
	return byte_buffer{bytes, tailroom};
}

inline bool failed_with (std::error_code error, turner::errc expected)
{
	return error == turner::make_error_code(expected);
}

template <auto Attribute, typename Reader>
void read_attribute (const Reader &reader, const auto &list_value, std::span<const std::byte> message)
{
	using value_type = typename decltype(Attribute)::value_type;
	using native_type = typename value_type::native_type;

	const auto single = reader.read(Attribute);
	require(same(list_value, single));
	if (single)
	{
		check_value(*single, message);
	}

	const auto each = reader.read_each(Attribute);
	if (each.begin() != each.end())
	{
		require(same(*each.begin(), single));
		for (const auto value: each)
		{
			if (value)
			{
				check_value(*value, message);
			}
		}
	}
	else
	{
		require(!single && failed_with(single.error(), turner::errc::attribute_not_found));
	}

	if constexpr (!std::is_empty_v<native_type> && std::is_default_constructible_v<native_type>)
	{
		const auto fallback = reader.read_or(Attribute, native_type{});
		if (single || !failed_with(single.error(), turner::errc::attribute_not_found))
		{
			require(same(fallback, single));
		}
		else
		{
			require(fallback.has_value());
		}
	}
}

template <auto... Attribute, typename Reader>
void read_attributes (const Reader &reader, std::span<const std::byte> message)
{
	constexpr auto list = turner::attributes<Attribute...>;
	if (message.size() <= read_check_limit)
	{
		const auto values = reader.read(list);
		[&]<size_t... I>(std::index_sequence<I...>)
		{
			(read_attribute<Attribute>(reader, std::get<I>(values), message), ...);
		}(std::index_sequence_for<decltype(Attribute)...>{});
	}

	std::vector<uint16_t> types;
	for (const auto attribute: reader.wire_attributes())
	{
		types.push_back(attribute.type);
		check_view(attribute.value, message);
	}
	for (const auto type: reader.unknown(list))
	{
		require(std::ranges::find(types, type) != types.end());
		require(turner::__wire::comprehension_required(type));
		require(((type != decltype(Attribute)::type) && ...));
	}

	touch(std::as_bytes(std::span{reader.transaction_id()}));
}

template <typename Reader, typename Key>
void check_verify (const Reader &reader, Key &key)
{
	const auto result = reader.verify(key);
	if (result)
	{
		require(reader.integrity() != turner::integrity_algorithm::none);
		return;
	}
	require(failed_with(result.error(), turner::errc::integrity_missing)
		|| failed_with(result.error(), turner::errc::integrity_mismatch));
	if (reader.integrity() == turner::integrity_algorithm::none)
	{
		require(failed_with(result.error(), turner::errc::integrity_missing));
	}
}

template <typename Reader, typename... Key>
void check_integrity (const Reader &reader, Key &...keys)
{
	(check_verify(reader, keys), ...);
}

template <typename Protocol, auto Request, auto Response, typename Transition, typename Check>
void check_transition (std::span<const std::byte> message, Transition transition, Check check)
{
	if (turner::__wire::type_and_length::read(message).type != Request.type)
	{
		return;
	}

	constexpr size_t tailroom = 512;
	auto buffer = copy_of(message, tailroom);
	const auto parsed = Protocol::parse(buffer, message.size());
	require(parsed.has_value());

	auto request = parsed->as(Request);
	require(request.has_value());
	const auto transaction_id = request->transaction_id();

	const auto response = transition(std::move(*request));
	require(response.has_value());

	auto copy = copy_of(*response);
	const auto reparsed = Protocol::parse(copy, copy.size());
	require(reparsed.has_value() && reparsed->size() == copy.size());
	const auto reader = reparsed->as(Response);
	require(reader.has_value());
	require(reader->transaction_id() == transaction_id);
	check(*reader);
}

template <typename Protocol, auto Request, auto Success, auto Error, typename Key>
void check_transitions (std::span<const std::byte> message, Key &key, turner::integrity_algorithm algorithm)
{
	check_transition<Protocol, Request, Success>(
		message,
		[](auto &&request)
		{
			return std::move(request).to_success().finish();
		},
		[](const auto &response)
		{
			require(response.integrity() == turner::integrity_algorithm::none);
		}
	);

	check_transition<Protocol, Request, Success>(
		message,
		[&](auto &&request)
		{
			return std::move(request).to_success().finish(key);
		},
		[&](const auto &response)
		{
			require(response.integrity() == algorithm);
			require(response.verify(key).has_value());
		}
	);

	check_transition<Protocol, Request, Error>(
		message,
		[&](auto &&request)
		{
			return std::move(request).to_error(turner::protocol_errc::bad_request).finish(key);
		},
		[&](const auto &response)
		{
			const auto error = response.read(Protocol::error_code);
			require(error && *error == turner::protocol_errc::bad_request);
			require(response.integrity() == algorithm);
			require(response.verify(key).has_value());
		}
	);

	if constexpr (requires { Protocol::fingerprint; })
	{
		check_transition<Protocol, Request, Success>(
			message,
			[](auto &&request)
			{
				return std::move(request).to_success().finish(Protocol::fingerprint);
			},
			[](const auto &response)
			{
				require(response.integrity() == turner::integrity_algorithm::none);
			}
		);

		check_transition<Protocol, Request, Success>(
			message,
			[&](auto &&request)
			{
				return std::move(request).to_success().finish(key, Protocol::fingerprint);
			},
			[&](const auto &response)
			{
				require(response.integrity() == algorithm);
				require(response.verify(key).has_value());
			}
		);
	}
}

template <typename Protocol>
std::span<const std::byte> parse_twice (std::span<const std::byte> input, byte_buffer &buffer)
{
	buffer = copy_of(input);
	const auto parsed = Protocol::parse(buffer, input.size());

	auto tailed = copy_of(input, 16);
	const auto tailed_parsed = Protocol::parse(tailed, input.size());

	require(same_bytes(buffer, input));
	require(same_bytes(std::span{tailed}.first(input.size()), input));

	require(parsed.has_value() == tailed_parsed.has_value());
	if (!parsed)
	{
		require(parsed.error() == tailed_parsed.error());
		return {};
	}
	require(parsed->size() <= input.size());
	require(parsed->size() == tailed_parsed->size());
	require(parsed->type() == tailed_parsed->type());
	return std::span{buffer}.first(parsed->size());
}

} // namespace turner_fuzz
