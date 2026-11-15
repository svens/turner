#pragma once

/**
 * \file turner/codec/method.hpp
 * Protocol methods and their messages
 */

#include <bit>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace turner
{

/// Message class. Values combine with operator| into the set of classes a method supports.
enum class message_class: uint8_t
{
	request = 1, ///< Request
	indication = 2, ///< Indication
	success = 4, ///< Success response
	error = 8, ///< Error response
	request_response = request | success | error, ///< Request with success and error responses
};

/// Return union of message class sets \a a and \a b
constexpr message_class operator| (message_class a, message_class b) noexcept
{
	return static_cast<message_class>(std::to_underlying(a) | std::to_underlying(b));
}

namespace __method
{

constexpr bool has (message_class set, message_class flag) noexcept
{
	return (std::to_underlying(set) & std::to_underlying(flag)) != 0;
}

constexpr bool has_single_initiating (message_class set) noexcept
{
	return has(set, message_class::request) != has(set, message_class::indication);
}

constexpr message_class initiating (message_class set) noexcept
{
	return has(set, message_class::request) ? message_class::request : message_class::indication;
}

constexpr bool is_valid (uint16_t method, message_class set) noexcept
{
	return method <= 0x0fff
		&& std::to_underlying(set) <= 0x0f
		&& has(set, message_class::request | message_class::indication)
		&& (has(set, message_class::request) || !has(set, message_class::success | message_class::error))
	;
}

constexpr uint16_t class_bits (message_class c) noexcept
{
	return static_cast<uint16_t>(
		(has(message_class::indication | message_class::error, c) ? 0b0000'0000'0001'0000 : 0)
		| (has(message_class::success | message_class::error, c) ? 0b0000'0001'0000'0000 : 0)
	);
}

constexpr uint16_t interleave (uint16_t method, message_class c) noexcept
{
	return static_cast<uint16_t>(
		(method & 0b0000'0000'0000'1111)
		| ((method & 0b0000'0000'0111'0000) << 1)
		| ((method & 0b0000'1111'1000'0000) << 2)
		| class_bits(c)
	);
}

template <typename Protocol, uint16_t Method, message_class Class>
constexpr uint16_t wire_type = interleave(Method, Class);

} // namespace __method

/**
 * Single message of \a Protocol method \a Method: the one with class \a Class out of the method's
 * supported \a Classes.
 */
template <typename Protocol, uint16_t Method, message_class Class, message_class Classes>
struct message_type
{
	static_assert(
		__method::is_valid(Method, Classes),
		"Method must fit into 12 bits; Classes must have request or indication, and request if it has responses"
	);
	static_assert(std::has_single_bit(std::to_underlying(Class)), "Class must name single message class");
	static_assert(__method::has(Classes, Class), "method does not support Class");

	/// Protocol that defines the message
	using protocol_type = Protocol;

	/// Method number
	static constexpr uint16_t method = Method;

	/// Message type as it appears in message header (host byte order)
	static constexpr uint16_t type = __method::wire_type<Protocol, Method, Class>;

	/// Set of message classes the method supports
	static constexpr message_class classes = Classes;
};

namespace __method
{

template <typename Protocol, uint16_t Method, message_class Classes, bool = has(Classes, message_class::request)>
struct with_request
{ };

template <typename Protocol, uint16_t Method, message_class Classes>
struct with_request<Protocol, Method, Classes, true>
{
	static constexpr message_type<Protocol, Method, message_class::request, Classes> request{};
};

template <typename Protocol, uint16_t Method, message_class Classes, bool = has(Classes, message_class::indication)>
struct with_indication
{ };

template <typename Protocol, uint16_t Method, message_class Classes>
struct with_indication<Protocol, Method, Classes, true>
{
	static constexpr message_type<Protocol, Method, message_class::indication, Classes> indication{};
};

template <typename Protocol, uint16_t Method, message_class Classes, bool = has(Classes, message_class::success)>
struct with_success
{ };

template <typename Protocol, uint16_t Method, message_class Classes>
struct with_success<Protocol, Method, Classes, true>
{
	static constexpr message_type<Protocol, Method, message_class::success, Classes> success{};
};

template <typename Protocol, uint16_t Method, message_class Classes, bool = has(Classes, message_class::error)>
struct with_error
{ };

template <typename Protocol, uint16_t Method, message_class Classes>
struct with_error<Protocol, Method, Classes, true>
{
	static constexpr message_type<Protocol, Method, message_class::error, Classes> error{};
};

template <typename Protocol, uint16_t Method, message_class Classes, bool = has_single_initiating(Classes)>
struct with_type
{ };

template <typename Protocol, uint16_t Method, message_class Classes>
struct with_type<Protocol, Method, Classes, true>
{
	static constexpr uint16_t type = message_type<Protocol, Method, initiating(Classes), Classes>::type;
};

} // namespace __method

/**
 * \a Protocol method \a Method supporting message \a Classes.
 *
 * For each class in \a Classes there is static member naming that message: \c request, \c indication,
 * \c success, \c error. Method with single initiating class (request or indication, not both) is itself
 * usable as that message and has member \c type with its message type value.
 */
template <typename Protocol, uint16_t Method, message_class Classes>
struct method_type
	: __method::with_request<Protocol, Method, Classes>
	, __method::with_indication<Protocol, Method, Classes>
	, __method::with_success<Protocol, Method, Classes>
	, __method::with_error<Protocol, Method, Classes>
	, __method::with_type<Protocol, Method, Classes>
{
	static_assert(
		__method::is_valid(Method, Classes),
		"Method must fit into 12 bits; Classes must have request or indication, and request if it has responses"
	);

	/// Protocol that defines the method
	using protocol_type = Protocol;

	/// Method number
	static constexpr uint16_t method = Method;

	/// Set of message classes the method supports
	static constexpr message_class classes = Classes;
};

/// \a Protocol method \a Method supporting message \a Classes
template <typename Protocol, uint16_t Method, message_class Classes>
constexpr method_type<Protocol, Method, Classes> method{};

/// \a Protocol method \a Method with request, success response and error response
template <typename Protocol, uint16_t Method>
constexpr auto request = method<Protocol, Method, message_class::request_response>;

/// \a Protocol method \a Method with indication only
template <typename Protocol, uint16_t Method>
constexpr auto indication = method<Protocol, Method, message_class::indication>;

namespace __method
{

template <typename T>
struct message_of
{ };

template <typename Protocol, uint16_t Method, message_class Class, message_class Classes>
struct message_of<message_type<Protocol, Method, Class, Classes>>
{
	using type = message_type<Protocol, Method, Class, Classes>;
};

template <typename Protocol, uint16_t Method, message_class Classes>
	requires(has_single_initiating(Classes))
struct message_of<method_type<Protocol, Method, Classes>>
{
	using type = message_type<Protocol, Method, initiating(Classes), Classes>;
};

} // namespace __method

/**
 * Message that entity \a T denotes: \a T itself if it is message_type, or the initiating message if
 * \a T is method_type with single initiating class. Otherwise ill-formed.
 */
template <typename T>
using message_of = typename __method::message_of<std::remove_cvref_t<T>>::type;

/// True if entity \a T denotes single message. \see message_of
template <typename T>
concept message = requires
{
	typename message_of<T>;
};

} // namespace turner
