#include <turner/error.hpp>


__turner_begin


namespace {


inline constexpr const char *to_message (errc e) noexcept
{
  switch (e)
  {
    case errc::insufficient_header_data:
      return "insufficient header data";
    case errc::insufficient_payload_data:
      return "insufficient payload data";
    case errc::invalid_message_type:
      return "invalid message type";
    case errc::invalid_message_length:
      return "invalid message length";
    case errc::invalid_message_cookie:
      return "invalid message cookie";
    case errc::unexpected_message_type:
      return "unexpected message type";
  }
  return "unknown error";
}


class category_impl_t
  : public std::error_category
{
public:

  const char *name () const noexcept final override
  {
    return "turner";
  }

  std::string message (int value) const final override
  {
    return to_message(static_cast<errc>(value));
  }
};

} // namespace


const std::error_category &category () noexcept
{
  static const category_impl_t cat_{};
  return cat_;
}


__turner_end
