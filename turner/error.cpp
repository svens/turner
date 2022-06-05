#include <turner/error.hpp>
#include <string>


__turner_begin


namespace {

constexpr std::string_view as_view (errc ec) noexcept
{
	switch (ec)
	{
		#define __turner_errc_case(code, message) case turner::errc::code: return message;
		__turner_errc(__turner_errc_case)
		#undef __turner_errc_case
	}
	return "unknown";
}

} // namespace


const std::error_category &error_category () noexcept
{
	struct error_category_impl: std::error_category
	{
		[[nodiscard]] const char *name () const noexcept final
		{
			return "turner";
		}

		[[nodiscard]] std::string message (int ec) const final
		{
			return std::string{as_view(static_cast<errc>(ec))};
		}
	};
	static const error_category_impl impl{};
	return impl;
}


__turner_end
