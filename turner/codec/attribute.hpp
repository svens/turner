#pragma once

/**
 * \file turner/codec/attribute.hpp
 * Protocol attributes
 */

#include <turner/codec/__wire.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>

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
		attributes_ = attributes_.subspan(sizeof(__wire::type_and_length) + padded_length);
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
		return pal::packed_as<__wire::type_and_length>(attributes_.first<sizeof(__wire::type_and_length)>());
	}
};

} // namespace turner
