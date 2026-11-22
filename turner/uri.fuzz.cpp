#include <turner/uri.hpp>
#include <pal/fuzz.hpp>
#include <pal/net/ip/address_v4.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <pal/uri.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace
{

using turner::uri_view;

constexpr size_t max_input = 256;
constexpr size_t build_header_size = 4;

void require (bool condition)
{
	if (!condition)
	{
		std::abort();
	}
}

uint8_t to_byte (char c)
{
	return static_cast<uint8_t>(c);
}

uint8_t byte_at (std::string_view blob, size_t index)
{
	return to_byte(blob[index]);
}

std::string require_to_chars (const uri_view &value)
{
	std::string formatted(value.max_size(), '\0');
	const auto [ptr, ec] = value.to_chars(formatted.data(), formatted.data() + formatted.size());
	require(ec == std::errc{});
	formatted.resize(static_cast<size_t>(ptr - formatted.data()));

	std::string exact(formatted.size(), '\0');
	const auto [exact_ptr, exact_ec] = value.to_chars(exact.data(), exact.data() + exact.size());
	require(exact_ec == std::errc{} && exact_ptr == exact.data() + exact.size() && exact == formatted);

	std::string small(formatted.size() - 1, '\0');
	const auto [small_ptr, small_ec] = value.to_chars(small.data(), small.data() + small.size());
	require(small_ec == std::errc::value_too_large && small_ptr == small.data() + small.size());

	return formatted;
}

std::string roundtrip (const uri_view &value)
{
	auto formatted = require_to_chars(value);
	const auto reparsed = uri_view::make(formatted);
	require(reparsed && *reparsed == value);
	return formatted;
}

bool try_roundtrip (const uri_view &value)
{
	std::string probe(value.max_size(), '\0');
	const auto [ptr, ec] = value.to_chars(probe.data(), probe.data() + probe.size());
	if (ec != std::errc{})
	{
		require(ec == std::errc::invalid_argument && ptr == probe.data() + probe.size());
		return false;
	}
	roundtrip(value);
	return true;
}

void parse (std::string_view blob)
{
	uri_view value;
	const auto [ptr, ec] = value.from_chars(blob.data(), blob.data() + blob.size());
	const auto made = uri_view::make(blob);
	require((ec == std::errc{}) == made.has_value());
	if (!made)
	{
		return;
	}
	require(ptr == blob.data() + blob.size() && *made == value);

	const auto formatted = roundtrip(*made);
	const auto *address = std::get_if<pal::net::ip::address>(&made->host.value);
	if (address == nullptr)
	{
		require(formatted.size() <= blob.size());
	}
	if (address == nullptr || address->is_v4())
	{
		const auto generic = pal::uri_view::make(blob);
		require(generic && !generic->host);
		const auto question = blob.find('?');
		if (question == blob.npos)
		{
			require(!generic->query);
		}
		else
		{
			require(generic->query == blob.substr(question + 1));
		}
	}
}

void build (std::string_view blob)
{
	if (blob.size() < build_header_size)
	{
		return;
	}

	const auto text = blob.substr(build_header_size);
	uri_view value{
		.scheme = static_cast<turner::uri_scheme>(byte_at(blob, 0) % 4),
		.host = {.value = text},
		.port = pal::net::ip::port_type{static_cast<uint16_t>(byte_at(blob, 2) << 8 | byte_at(blob, 3))},
		.transport = static_cast<turner::transport>(byte_at(blob, 1) % 2),
	};

	const auto host = pal::uri_host::make(text);
	const auto accepted = try_roundtrip(value);
	require(accepted == (!text.empty() && host && *host == value.host));
	if (!accepted)
	{
		const auto uri_text = "stun:" + std::string{text};
		const auto parsed = uri_view::make(uri_text);
		require(!parsed || parsed->host != value.host);
	}

	pal::net::ip::address_v4::bytes_type v4;
	if (text.size() >= v4.size())
	{
		std::ranges::transform(text.substr(0, v4.size()), v4.begin(), to_byte);
		value.host = {.value = pal::net::ip::make_address_v4(v4)};
		require(try_roundtrip(value));
	}

	pal::net::ip::address_v6::bytes_type v6;
	if (text.size() > v6.size())
	{
		std::ranges::transform(text.substr(0, v6.size()), v6.begin(), to_byte);
		const auto scope_id = byte_at(text, v6.size());
		value.host = {.value = pal::net::ip::make_address_v6(v6, scope_id)};
		require(try_roundtrip(value) == (scope_id == 0));
	}
}

} // namespace

extern "C" int LLVMFuzzerInitialize (int *argc, char ***argv)
{
	pal::fuzz::init(argc, argv, max_input);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, std::size_t size)
{
	const auto blob = std::string_view{reinterpret_cast<const char *>(data), size};
	parse(blob);
	build(blob);
	return 0;
}
