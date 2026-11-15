#include <turner/codec/protocol_error.hpp>
#include <string>
#include <string_view>

namespace turner
{

namespace
{

constexpr std::string_view as_view (int ec) noexcept
{
	switch (ec)
	{
#define __turner_protocol_errc_case(Value, Code, Message) case Value: return Message;
		__turner_protocol_errc(__turner_protocol_errc_case)
#undef __turner_protocol_errc_case
		default: return "unknown";
	}
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
