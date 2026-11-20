#pragma once

/**
 * \file turner/codec/attribute.hpp
 * Protocol attributes
 */

#include <turner/codec/__wire.hpp>
#include <pal/result.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace turner
{

/// Requirements for type that converts between attribute value bytes and native type.
///
/// \c min_size and \c max_size bound value size on wire: reader and writer check them, \c read and \c write
/// get value span of valid size. First argument of \c read and \c write is whole message.
template <typename T>
concept attribute_value = requires(
	std::span<const std::byte> message,
	std::span<const std::byte> value,
	std::span<std::byte> out,
	const typename T::native_type &native)
{
	{ T::min_size } -> std::convertible_to<size_t>;
	{ T::max_size } -> std::convertible_to<size_t>;
	{ T::size(native) } noexcept -> std::same_as<size_t>;
	{ T::read(message, value) } noexcept -> std::same_as<pal::result<typename T::native_type>>;
	{ T::write(message, out, native) } noexcept -> std::same_as<void>;
};

/// \a Protocol attribute \a Type with value handled by \a Value
template <typename Protocol, attribute_value Value, uint16_t Type>
struct attribute_type
{
	static_assert(Value::min_size <= Value::max_size, "Value min_size exceeds max_size");
	static_assert(Value::max_size <= 0xffff, "Value max_size exceeds what attribute length can express");

	/// Protocol that defines the attribute
	using protocol_type = Protocol;

	/// Attribute value type
	using value_type = Value;

	/// Attribute type as it appears in message (host byte order)
	static constexpr uint16_t type = Type;
};

/// \a Protocol attribute \a Type with value handled by \a Value
template <typename Protocol, attribute_value Value, uint16_t Type>
constexpr attribute_type<Protocol, Value, Type> attribute{};

template <typename... Attribute>
struct attribute_list;

namespace __attribute_list
{

template <typename T>
struct members;

template <typename Protocol, typename Value, uint16_t Type>
struct members<attribute_type<Protocol, Value, Type>>
{
	using type = attribute_list<attribute_type<Protocol, Value, Type>>;
};

template <typename... Attribute>
struct members<attribute_list<Attribute...>>
{
	using type = attribute_list<Attribute...>;
};

template <auto Entry>
concept entry = requires
{
	typename members<std::remove_cvref_t<decltype(Entry)>>::type;
};

template <typename... List>
struct concat
{
	using type = attribute_list<>;
};

template <typename... Attribute>
struct concat<attribute_list<Attribute...>>
{
	using type = attribute_list<Attribute...>;
};

template <typename... A, typename... B, typename... Rest>
struct concat<attribute_list<A...>, attribute_list<B...>, Rest...>: concat<attribute_list<A..., B...>, Rest...>
{
};

template <auto... Entry>
using flatten = typename concat<typename members<std::remove_cvref_t<decltype(Entry)>>::type...>::type;

template <typename Protocol, typename... Rest>
struct most_derived
{
	using type = Protocol;
};

template <typename A, typename B, typename... Rest>
struct most_derived<A, B, Rest...>: most_derived<std::conditional_t<std::derived_from<A, B>, A, B>, Rest...>
{
};

template <typename... Attribute>
using protocol_of = typename most_derived<typename Attribute::protocol_type...>::type;

template <uint16_t... Type>
constexpr bool distinct () noexcept
{
	constexpr uint16_t types[] = {Type...};
	for (size_t i = 0; i < sizeof...(Type); ++i)
	{
		for (size_t j = i + 1; j < sizeof...(Type); ++j)
		{
			if (types[i] == types[j])
			{
				return false;
			}
		}
	}
	return true;
}

template <typename List>
constexpr bool non_empty = false;

template <typename... Attribute>
constexpr bool non_empty<attribute_list<Attribute...>> = sizeof...(Attribute) > 0;

template <typename List>
constexpr bool one_protocol = false;

template <typename... Attribute>
	requires(sizeof...(Attribute) > 0)
constexpr bool one_protocol<attribute_list<Attribute...>> =
	(std::derived_from<protocol_of<Attribute...>, typename Attribute::protocol_type> && ...);

template <typename List>
constexpr bool distinct_types = false;

template <typename... Attribute>
	requires(sizeof...(Attribute) > 0)
constexpr bool distinct_types<attribute_list<Attribute...>> = distinct<Attribute::type...>();

template <typename... Attribute>
constexpr bool contains (uint16_t type) noexcept
{
	return ((type == Attribute::type) || ...);
}

} // namespace __attribute_list

/// List of attributes, created with \ref attributes. Its protocol is the most derived protocol of its
/// attributes; every attribute is defined by that protocol or its base.
template <typename... Attribute>
struct attribute_list
{
	/// Most derived protocol of listed attributes
	using protocol_type = __attribute_list::protocol_of<Attribute...>;
};

/// List of \a Entry attributes and attribute lists, flattened in declaration order. Fails to compile if the list:
/// - is empty
/// - lists attribute type more than once
/// - mixes attributes of protocols not derived from one another
template <auto... Entry>
	requires(__attribute_list::entry<Entry> && ...)
		&& __attribute_list::non_empty<__attribute_list::flatten<Entry...>>
		&& __attribute_list::one_protocol<__attribute_list::flatten<Entry...>>
		&& __attribute_list::distinct_types<__attribute_list::flatten<Entry...>>
constexpr __attribute_list::flatten<Entry...> attributes{};

/// Attribute as it appears in message
struct wire_attribute
{
	/// Attribute type (host byte order)
	uint16_t type;

	/// Attribute value without padding
	std::span<const std::byte> value;
};

namespace __iterator
{

template <typename T>
struct arrow_proxy
{
	T value;

	[[nodiscard]] const T *operator-> () const noexcept
	{
		return &value;
	}
};

} // namespace __iterator

/// Iterator over attributes of validated \a Protocol message. Elements are returned by value: with
/// algorithms that need more than single pass, use those of \c std::ranges.
template <typename Protocol>
class wire_attribute_iterator
{
public:

	/// Iterator difference type
	using difference_type = std::ptrdiff_t;

	/// Iterator value type
	using value_type = wire_attribute;

	/// Construct iterator that refers to no attribute
	wire_attribute_iterator () noexcept = default;

	/// Construct iterator to first attribute in \a attributes
	explicit wire_attribute_iterator (std::span<const std::byte> attributes) noexcept
		: attributes_{attributes}
	{
	}

	/// Return current attribute
	[[nodiscard]] wire_attribute operator* () const noexcept
	{
		const auto &h = header();
		return {.type = h.type, .value = attributes_.subspan(sizeof(h), h.length)};
	}

	/// Return current attribute, for member access
	[[nodiscard]] __iterator::arrow_proxy<wire_attribute> operator-> () const noexcept
	{
		return {**this};
	}

	/// Advance to next attribute
	wire_attribute_iterator &operator++ () noexcept
	{
		const auto padded_length = __wire::align_up(header().length, Protocol::attribute_alignment);
		attributes_ = attributes_.subspan(__wire::attribute_header_size + padded_length);
		return *this;
	}

	/// Advance to next attribute, return iterator to current one
	wire_attribute_iterator operator++ (int) noexcept
	{
		auto current = *this;
		++*this;
		return current;
	}

	/// Return true if \a a and \a b refer to same attribute
	friend bool operator== (const wire_attribute_iterator &a, const wire_attribute_iterator &b) noexcept
	{
		return a.attributes_.data() == b.attributes_.data();
	}

private:

	std::span<const std::byte> attributes_;

	[[nodiscard]] const __wire::type_and_length &header () const noexcept
	{
		return __wire::type_and_length::read(attributes_);
	}
};

} // namespace turner
