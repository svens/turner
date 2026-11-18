#pragma once

/**
 * \file turner/codec/reader.hpp
 * Parsed message and typed message reader
 */

#include <turner/codec/__wire.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/protocol_error.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/writer.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

namespace turner
{

template <typename Protocol>
class parsed_message;

namespace __reader
{

template <typename Protocol>
const typename Protocol::transaction_id &transaction_id (std::span<std::byte> buffer) noexcept
{
	using id_type = typename Protocol::transaction_id;
	return pal::packed_as<id_type>(buffer.subspan<Protocol::transaction_id_offset, sizeof(id_type)>());
}

template <typename Request, message_class Class>
using response_writer = basic_message_writer<
	message_type<typename Request::protocol_type, Request::method, Class, Request::classes>
>;

template <attribute_value Value>
[[nodiscard]] pal::result<typename Value::native_type> read_value (
	std::span<const std::byte> message,
	std::span<const std::byte> value) noexcept
{
	if (value.size() < Value::min_size || value.size() > Value::max_size)
	{
		return make_unexpected(errc::unexpected_attribute_length);
	}
	return Value::read(message, value);
}

template <typename List>
struct list_values;

template <typename... Attribute>
struct list_values<attribute_list<Attribute...>>
{
	using type = std::tuple<pal::result<typename Attribute::value_type::native_type>...>;
};

template <typename... Attribute, typename Attributes, size_t... I>
[[nodiscard]] typename list_values<attribute_list<Attribute...>>::type read_list (
	attribute_list<Attribute...>,
	std::span<const std::byte> message,
	Attributes attributes,
	std::index_sequence<I...>) noexcept
{
	using types = std::tuple<Attribute...>;

	typename list_values<attribute_list<Attribute...>>::type values{
		(static_cast<void>(I), make_unexpected(errc::attribute_not_found))...
	};
	bool seen[sizeof...(I)]{};
	size_t remaining = sizeof...(I);

	for (const auto attribute: attributes)
	{
		const auto read_first = [&]<size_t N>(std::integral_constant<size_t, N>) noexcept
		{
			using type = std::tuple_element_t<N, types>;
			if (attribute.type != type::type)
			{
				return false;
			}
			if (!seen[N])
			{
				seen[N] = true;
				std::get<N>(values) = read_value<typename type::value_type>(message, attribute.value);
				--remaining;
			}
			return true;
		};
		static_cast<void>((read_first(std::integral_constant<size_t, I>{}) || ...));
		if (remaining == 0)
		{
			break;
		}
	}
	return values;
}

} // namespace __reader

/// Iterator over values of attributes of type \a Type in validated \a Protocol message. Each element is
/// result of reading one such attribute with \a Value. Elements are returned by value: with algorithms that
/// need more than single pass, use those of \c std::ranges.
template <typename Protocol, attribute_value Value, uint16_t Type>
class attribute_value_iterator
{
public:

	/// Iterator difference type
	using difference_type = std::ptrdiff_t;

	/// Iterator value type
	using value_type = pal::result<typename Value::native_type>;

	/// Construct iterator that refers to no attribute
	attribute_value_iterator () noexcept = default;

	/// Construct iterator to value of first attribute of type \a Type in \a attributes of \a message
	attribute_value_iterator (
		std::ranges::subrange<wire_attribute_iterator<Protocol>> attributes,
		std::span<const std::byte> message) noexcept
		: current_{attributes.begin()}
		, end_{attributes.end()}
		, message_{message}
	{
		skip_other_types();
	}

	/// Return result of reading current attribute
	[[nodiscard]] value_type operator* () const noexcept
	{
		return __reader::read_value<Value>(message_, current_->value);
	}

	/// Return result of reading current attribute, for member access
	[[nodiscard]] __iterator::arrow_proxy<value_type> operator-> () const noexcept
	{
		return {**this};
	}

	/// Advance to next attribute of type \a Type
	attribute_value_iterator &operator++ () noexcept
	{
		++current_;
		skip_other_types();
		return *this;
	}

	/// Advance to next attribute of type \a Type, return iterator to current one
	attribute_value_iterator operator++ (int) noexcept
	{
		auto current = *this;
		++*this;
		return current;
	}

	/// Return true if \a a and \a b refer to same attribute
	friend bool operator== (const attribute_value_iterator &a, const attribute_value_iterator &b) noexcept
	{
		return a.current_ == b.current_;
	}

private:

	wire_attribute_iterator<Protocol> current_;
	wire_attribute_iterator<Protocol> end_;
	std::span<const std::byte> message_;

	void skip_other_types () noexcept
	{
		while (current_ != end_ && current_->type != Type)
		{
			++current_;
		}
	}
};

/// Reader of validated message of type \a Message (message_type). Created by parsed_message::as().
template <typename Message>
class basic_message_reader
{
public:

	/// Protocol that defines the message
	using protocol_type = typename Message::protocol_type;

	/// Return message transaction ID. Reference is valid while its bytes in the buffer are unchanged.
	[[nodiscard]] const typename protocol_type::transaction_id &transaction_id () const noexcept
	{
		return __reader::transaction_id<protocol_type>(buffer_);
	}

	/// Iterator over message attributes
	using wire_attribute_iterator = turner::wire_attribute_iterator<protocol_type>;

	/// Iterator over values of every occurrence of \a Attribute in message
	template <auto Attribute>
	using attribute_value_iterator = turner::attribute_value_iterator<
		protocol_type,
		typename decltype(Attribute)::value_type,
		decltype(Attribute)::type
	>;

	/// Range of values of every occurrence of \a Attribute in message: what read_each() returns
	template <auto Attribute>
	using attribute_value_range = std::ranges::subrange<attribute_value_iterator<Attribute>>;

	/// Tuple of results of reading attributes of \a List, in list order: what read(list) returns
	template <auto List>
	using attribute_values = typename __reader::list_values<std::remove_cvref_t<decltype(List)>>::type;

	/// Return message attributes as they appear in message, in message order. Attributes end at first integrity
	/// attribute or FINGERPRINT: these and anything following them are not returned, here or by other readers
	/// of attributes.
	[[nodiscard]] std::ranges::subrange<wire_attribute_iterator> wire_attributes () const noexcept
	{
		const auto bytes = buffer_.first(protocol_type::header_size + attributes_end_)
			.subspan(__wire::attributes_offset<protocol_type>);
		const std::span<const std::byte> attributes = bytes;
		return {wire_attribute_iterator{attributes}, wire_attribute_iterator{attributes.last(0)}};
	}

	/// Return value of first occurrence of given attribute in message. Fails with errc::attribute_not_found
	/// if there is none, and with errc::unexpected_attribute_length or errc::unexpected_attribute_value if it
	/// is malformed.
	template <typename P, attribute_value V, uint16_t Type>
		requires usable_with<attribute_type<P, V, Type>, protocol_type>
	[[nodiscard]] pal::result<typename V::native_type> read (attribute_type<P, V, Type>) const noexcept
	{
		for (const auto attribute: wire_attributes())
		{
			if (attribute.type == Type)
			{
				return __reader::read_value<V>(message(), attribute.value);
			}
		}
		return make_unexpected(errc::attribute_not_found);
	}

	/// Return value of first occurrence of \a attribute in message, or \a fallback if there is none. Fails
	/// with errc::unexpected_attribute_length or errc::unexpected_attribute_value if the attribute is
	/// malformed. Not available for attribute without value (flag): use read() to test its presence.
	template <typename P, attribute_value V, uint16_t Type>
		requires usable_with<attribute_type<P, V, Type>, protocol_type>
			&& (!std::is_empty_v<typename V::native_type>)
	[[nodiscard]] pal::result<typename V::native_type> read_or (
		attribute_type<P, V, Type> attribute,
		const typename V::native_type &fallback) const noexcept
	{
		auto value = read(attribute);
		if (!value && value.error() == errc::attribute_not_found)
		{
			return fallback;
		}
		return value;
	}

	/// Not available: value of attribute refers to bytes it does not own, \a fallback temporary would be
	/// destroyed while returned value still refers to it.
	template <typename P, attribute_value V, uint16_t Type, std::ranges::range Fallback>
		requires usable_with<attribute_type<P, V, Type>, protocol_type>
			&& std::ranges::borrowed_range<typename V::native_type>
			&& (!std::ranges::borrowed_range<Fallback>)
			&& std::convertible_to<Fallback, typename V::native_type>
	pal::result<typename V::native_type> read_or (attribute_type<P, V, Type>, Fallback &&fallback) const = delete;

	/// Return range of values of every occurrence of given attribute in message, in message order. Each
	/// element is result of reading one occurrence: value, or errc::unexpected_attribute_length or
	/// errc::unexpected_attribute_value if that occurrence is malformed.
	template <typename P, attribute_value V, uint16_t Type>
		requires usable_with<attribute_type<P, V, Type>, protocol_type>
	[[nodiscard]] attribute_value_range<attribute<P, V, Type>> read_each (
		attribute_type<P, V, Type>) const noexcept
	{
		using iterator = attribute_value_iterator<attribute<P, V, Type>>;
		const auto attributes = wire_attributes();
		return {
			iterator{attributes, message()},
			iterator{{attributes.end(), attributes.end()}, message()},
		};
	}

	/// Return values of first occurrence of each attribute of \a list, in list order, reading message once.
	/// Each element is what read() returns for its attribute.
	template <typename... Attribute>
		requires usable_with<attribute_list<Attribute...>, protocol_type>
	[[nodiscard]] attribute_values<attribute_list<Attribute...>{}> read (
		attribute_list<Attribute...> list) const noexcept
	{
		return __reader::read_list(
			list,
			message(),
			wire_attributes(),
			std::make_index_sequence<sizeof...(Attribute)>{}
		);
	}

	/// Return comprehension-required attribute types present in message but not listed in given list, each
	/// once, in message order. Types beyond attribute_type_list::capacity are dropped.
	template <typename... Attribute>
		requires usable_with<attribute_list<Attribute...>, protocol_type>
	[[nodiscard]] attribute_type_list unknown (attribute_list<Attribute...>) const noexcept
	{
		attribute_type_list types;
		for (const auto attribute: wire_attributes())
		{
			if (types.size() == attribute_type_list::capacity)
			{
				break;
			}
			if (__wire::comprehension_required(attribute.type)
				&& !__attribute_list::contains<Attribute...>(attribute.type)
				&& std::ranges::find(types, attribute.type) == types.end())
			{
				types.push_back(attribute.type);
			}
		}
		return types;
	}

	/// Return algorithm of message integrity attribute. For STUN and TURN, MESSAGE-INTEGRITY counts only as first
	/// integrity attribute, and MESSAGE-INTEGRITY-SHA256 only as first one or directly following first
	/// MESSAGE-INTEGRITY, when it is preferred.
	[[nodiscard]] integrity_algorithm integrity () const noexcept
	{
		return protocol_type::integrity_type::algorithm(message(), attributes_end_);
	}

	/// Verify message integrity attribute with \a key. Fails with errc::integrity_missing if message has no
	/// integrity attribute of \a key algorithm that counts as described in integrity(), and with
	/// errc::integrity_mismatch if its value differs.
	template <typename P>
		requires std::derived_from<protocol_type, P>
	[[nodiscard]] pal::result<void> verify (integrity_key<P> &key) const noexcept
	{
		return protocol_type::integrity_type::verify(key.hmac_, message(), attributes_end_);
	}

	/// Return writer of success response built over this request, in whole buffer given to parse: message type
	/// is replaced and attributes are dropped; Magic Cookie and transaction ID stay. Afterwards parsed_message,
	/// every reader of this message and every attribute value read from it are dead; only references returned
	/// by transaction_id() stay valid. For reader in \c std::optional, call as
	/// \c std::move(*request).to_success().
	[[nodiscard]] __reader::response_writer<Message, message_class::success> to_success () && noexcept
		requires(__method::class_of<Message> == message_class::request
			&& __method::has(Message::classes, message_class::success))
	{
		return __reader::response_writer<Message, message_class::success>{buffer_};
	}

	/// As to_success(), for error response with ERROR-CODE \a error as first attribute. Reason of \a error must
	/// not refer to bytes of the buffer, and its code must be in range 300 to 699. finish() fails with
	/// errc::unexpected_attribute_length if reason is too long for protocol, and with errc::buffer_too_small
	/// if ERROR-CODE does not fit.
	[[nodiscard]] __reader::response_writer<Message, message_class::error> to_error (
		const protocol_error &error) && noexcept
		requires(__method::class_of<Message> == message_class::request
			&& __method::has(Message::classes, message_class::error))
	{
		return __reader::response_writer<Message, message_class::error>{buffer_}
			.write(protocol_type::error_code, error);
	}

private:

	std::span<std::byte> buffer_;
	uint16_t attributes_end_;

	basic_message_reader (std::span<std::byte> buffer, uint16_t attributes_end) noexcept
		: buffer_{buffer}
		, attributes_end_{attributes_end}
	{
	}

	[[nodiscard]] std::span<const std::byte> message () const noexcept
	{
		return buffer_.first(protocol_type::header_size + __wire::type_and_length::read(buffer_).length);
	}

	template <typename>
	friend class parsed_message;
};

/// Reader of message that \a Entity denotes: message, or method with single initiating class
template <auto Entity>
using message_reader = basic_message_reader<message_of<decltype(Entity)>>;

/// Validated \a Protocol message of yet unknown type. Created by \c Protocol::parse().
template <typename Protocol>
class parsed_message
{
public:

	/// Protocol the message was parsed as
	using protocol_type = Protocol;

	/// Return message size in bytes, header included
	[[nodiscard]] size_t size () const noexcept
	{
		return Protocol::header_size + __wire::type_and_length::read(buffer_).length;
	}

	/// Return message type as it appears in message header (host byte order)
	[[nodiscard]] uint16_t type () const noexcept
	{
		return __wire::type_and_length::read(buffer_).type;
	}

	/// Return message transaction ID. Reference is valid while its bytes in the buffer are unchanged.
	[[nodiscard]] const typename Protocol::transaction_id &transaction_id () const noexcept
	{
		return __reader::transaction_id<Protocol>(buffer_);
	}

	/// Return reader of message \a M if this is one, otherwise \c std::nullopt
	template <message M>
		requires usable_with<M, Protocol>
	[[nodiscard]] std::optional<basic_message_reader<message_of<M>>> as (M) const noexcept
	{
		if (type() == message_of<M>::type)
		{
			return basic_message_reader<message_of<M>>{buffer_, attributes_end_};
		}
		return std::nullopt;
	}

private:

	std::span<std::byte> buffer_;
	uint16_t attributes_end_;

	parsed_message (std::span<std::byte> buffer, uint16_t attributes_end) noexcept
		: buffer_{buffer}
		, attributes_end_{attributes_end}
	{
	}

	friend Protocol;
};

} // namespace turner
