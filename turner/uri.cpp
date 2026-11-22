#include <turner/uri.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace turner
{

namespace
{

constexpr std::array<std::string_view, 4> scheme_names{"stun", "stuns", "turn", "turns"};
static_assert(std::to_underlying(uri_scheme::stun) == 0);
static_assert(std::to_underlying(uri_scheme::stuns) == 1);
static_assert(std::to_underlying(uri_scheme::turn) == 2);
static_assert(std::to_underlying(uri_scheme::turns) == 3);

constexpr std::array<std::string_view, 2> transport_names{"udp", "tcp"};
static_assert(std::to_underlying(transport::datagram) == 0);
static_assert(std::to_underlying(transport::stream) == 1);
constexpr std::string_view transport_key = "?transport=";

template <size_t N>
consteval size_t longest (const std::array<std::string_view, N> &names) noexcept
{
	size_t size = 0;
	for (const auto name: names)
	{
		size = (std::max)(size, name.size());
	}
	return size;
}

constexpr size_t scheme_max_size = longest(scheme_names) + 1;
constexpr size_t port_max_size = sizeof(":65535") - 1;
constexpr size_t transport_max_size = transport_key.size() + longest(transport_names);

constexpr std::string_view head (std::string_view text, size_t n) noexcept
{
	text.remove_suffix(text.size() - (std::min)(n, text.size()));
	return text;
}

constexpr std::string_view tail (std::string_view text, size_t pos) noexcept
{
	text.remove_prefix((std::min)(pos, text.size()));
	return text;
}

constexpr char to_lower (char c) noexcept
{
	return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
}

constexpr bool equal_lowercase (std::string_view text, std::string_view lowercase) noexcept
{
	return std::ranges::equal(text, lowercase, {}, to_lower);
}

template <typename T, size_t N>
bool parse_name (std::string_view text, const std::array<std::string_view, N> &names, T &value) noexcept
{
	for (size_t i = 0; i < N; ++i)
	{
		if (equal_lowercase(text, names[i]))
		{
			value = static_cast<T>(i);
			return true;
		}
	}
	return false;
}

bool parse_port (std::string_view text, pal::net::ip::port_type &port) noexcept
{
	if (text.empty())
	{
		return true;
	}
	uint16_t value = 0;
	const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
	if (ec != std::errc{} || ptr != text.data() + text.size())
	{
		return false;
	}
	port = pal::net::ip::port_type{value};
	return true;
}

bool is_empty_reg_name (const pal::uri_host &host) noexcept
{
	const auto *reg_name = std::get_if<std::string_view>(&host.value);
	return reg_name != nullptr && reg_name->empty();
}

struct writer
{
	char *ptr;
	char *last;

	bool put_text (std::string_view text) noexcept
	{
		if (static_cast<size_t>(last - ptr) < text.size())
		{
			return false;
		}
		ptr = std::ranges::copy(text, ptr).out;
		return true;
	}

	bool put_host (const pal::uri_host &host) noexcept
	{
		const auto [end, ec] = host.to_chars(ptr, last);
		ptr = end;
		return ec == std::errc{};
	}

	bool put_port (pal::net::ip::port_type port) noexcept
	{
		if (!put_text(":"))
		{
			return false;
		}
		const auto [end, ec] = std::to_chars(ptr, last, std::to_underlying(port));
		ptr = end;
		return ec == std::errc{};
	}

	bool put_transport (transport value) noexcept
	{
		return put_text(transport_key) && put_text(transport_names[std::to_underlying(value)]);
	}
};

std::from_chars_result invalid_input (const char *first) noexcept
{
	return {.ptr = first, .ec = std::errc::invalid_argument};
}

std::to_chars_result fail (char *last, std::errc ec) noexcept
{
	return {.ptr = last, .ec = ec};
}

} // namespace

pal::result<uri_view> uri_view::make (std::string_view text) noexcept
{
	uri_view uri;
	if (uri.from_chars(text.data(), text.data() + text.size()).ec != std::errc{})
	{
		return pal::make_unexpected(std::errc::invalid_argument);
	}
	return uri;
}

std::from_chars_result uri_view::from_chars (const char *first, const char *last) noexcept
{
	std::string_view text{first, last};

	const auto colon = text.find(':');
	if (colon == text.npos || !parse_name(head(text, colon), scheme_names, scheme))
	{
		return invalid_input(first);
	}
	text = tail(text, colon + 1);

	const auto [host_end, host_ec] = host.from_chars(text.data(), text.data() + text.size());
	if (host_ec != std::errc{} || is_empty_reg_name(host))
	{
		return invalid_input(first);
	}
	text = tail(text, static_cast<size_t>(host_end - text.data()));

	port = default_port(scheme);
	transport = default_transport(scheme);

	if (text.starts_with(':'))
	{
		const auto port_text = head(tail(text, 1), text.find('?') - 1);
		if (!parse_port(port_text, port))
		{
			return invalid_input(first);
		}
		text = tail(text, 1 + port_text.size());
	}

	if (!text.empty()
		&& (!equal_lowercase(head(text, transport_key.size()), transport_key)
			|| !parse_name(tail(text, transport_key.size()), transport_names, transport)))
	{
		return invalid_input(first);
	}

	return {.ptr = last, .ec = {}};
}

std::to_chars_result uri_view::to_chars (char *first, char *last) const noexcept
{
	if (is_empty_reg_name(host) || !host.is_valid())
	{
		return fail(last, std::errc::invalid_argument);
	}

	writer out{.ptr = first, .last = last};
	const auto ok = out.put_text(scheme_names[std::to_underlying(scheme)])
		&& out.put_text(":")
		&& out.put_host(host)
		&& (port == default_port(scheme) || out.put_port(port))
		&& (transport == default_transport(scheme) || out.put_transport(transport))
	;

	if (!ok)
	{
		return fail(last, std::errc::value_too_large);
	}
	return {.ptr = out.ptr, .ec = {}};
}

size_t uri_view::max_size () const noexcept
{
	return scheme_max_size + host.max_size() + port_max_size + transport_max_size;
}

} // namespace turner
