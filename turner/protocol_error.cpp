#include <turner/protocol_error>
#include <string>


__turner_begin


namespace {

constexpr std::string_view as_view (protocol_errc ec) noexcept
{
	switch (ec)
	{
		#define __turner_errc_impl(value, symbol, message) case turner::protocol_errc::symbol: return message;
		__turner_protocol_errc(__turner_errc_impl)
		#undef __turner_errc_impl
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
			return std::string{as_view(static_cast<protocol_errc>(ec))};
		}
	};
	static const error_category_impl impl{};
	return impl;
}


__turner_end
