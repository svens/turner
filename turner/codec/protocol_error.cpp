#include <turner/codec/protocol_error.hpp>
#include <limits>
#include <string>
#include <string_view>

namespace turner
{

namespace
{

constexpr std::string_view as_view (int ec) noexcept
{
	if (0 <= ec && ec <= std::numeric_limits<uint16_t>::max())
	{
		if (const auto phrase = reason_phrase(static_cast<protocol_errc>(ec)); !phrase.empty())
		{
			return phrase;
		}
	}
	return "unknown";
}

} // namespace

const std::error_category &protocol_error_category () noexcept
{
	struct error_category_impl: std::error_category
	{
		[[nodiscard]] const char *name () const noexcept final
		{
			return "turner::protocol";
		}

		[[nodiscard]] std::string message (int ec) const final
		{
			return std::string{as_view(ec)};
		}
	};
	static const error_category_impl impl{};
	return impl;
}

} // namespace turner
