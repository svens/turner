#include <turner/codec/method.hpp>
#include <turner/codec/protocol.hpp>
#include <concepts>
#include <type_traits>

namespace
{

using turner::message_class;

struct proto
{ };

struct derived_proto: proto
{ };

struct other_proto
{ };

constexpr auto all = turner::method<proto, 0x001, message_class::request_response | message_class::indication>;
constexpr auto req = turner::request<proto, 0x003>;
constexpr auto ind = turner::indication<proto, 0x006>;
constexpr auto req_only = turner::method<proto, 0x004, message_class::request>;

template <auto M>
concept has_request = requires
{
	M.request;
};

template <auto M>
concept has_indication = requires
{
	M.indication;
};

template <auto M>
concept has_success = requires
{
	M.success;
};

template <auto M>
concept has_error = requires
{
	M.error;
};

template <auto M>
concept has_type = requires
{
	M.type;
};

static_assert(has_request<all> && has_indication<all> && has_success<all> && has_error<all>);
static_assert(has_request<req> && !has_indication<req> && has_success<req> && has_error<req>);
static_assert(!has_request<ind> && has_indication<ind> && !has_success<ind> && !has_error<ind>);
static_assert(has_request<req_only> && !has_indication<req_only> && !has_success<req_only> && !has_error<req_only>);

static_assert(!has_type<all>);
static_assert(has_type<req> && has_type<ind> && has_type<req_only>);
static_assert(has_type<req.success> && has_type<all.indication>);
static_assert(!has_success<req.request>);

static_assert(all.request.type == 0x0001);
static_assert(all.indication.type == 0x0011);
static_assert(all.success.type == 0x0101);
static_assert(all.error.type == 0x0111);
static_assert(req.type == 0x0003 && req.request.type == 0x0003);
static_assert(req.success.type == 0x0103);
static_assert(req.error.type == 0x0113);
static_assert(ind.type == 0x0016 && ind.indication.type == 0x0016);
static_assert(req_only.type == 0x0004);

static_assert(turner::request<proto, 0x010>.type == 0x0020);
static_assert(turner::request<proto, 0x080>.type == 0x0200);
static_assert(turner::request<proto, 0xfff>.type == 0x3eef);
static_assert(turner::request<proto, 0xfff>.error.type == 0x3fff);

static_assert(req.method == 0x003 && req.success.method == 0x003);
static_assert(req.request.classes == req.classes);
static_assert(req.classes == (message_class::request | message_class::success | message_class::error));

static_assert(turner::message<decltype(req)>);
static_assert(turner::message<decltype(ind)>);
static_assert(turner::message<decltype(req_only)>);
static_assert(turner::message<decltype(req.success)>);
static_assert(turner::message<decltype(all.request)>);
static_assert(!turner::message<decltype(all)>);
static_assert(!turner::message<int>);

static_assert(std::same_as<turner::message_of<decltype(req)>, turner::message_of<decltype(req.request)>>);
static_assert(std::same_as<turner::message_of<decltype(ind)>, turner::message_of<decltype(ind.indication)>>);
static_assert(std::same_as<turner::message_of<decltype((req))>, turner::message_of<decltype(req.request)>>);
static_assert(!std::same_as<turner::message_of<decltype(req)>, turner::message_of<decltype(req.success)>>);
static_assert(turner::message_of<decltype(req)>::type == 0x0003);

template <turner::message auto M>
constexpr auto type_of = turner::message_of<decltype(M)>::type;

static_assert(type_of<req> == type_of<req.request>);
static_assert(type_of<req.error> == 0x0113);
static_assert(type_of<all.indication> == 0x0011);

static_assert(turner::usable_with<decltype(req), proto>);
static_assert(turner::usable_with<decltype(req), derived_proto>);
static_assert(turner::usable_with<decltype(req.success), derived_proto>);
static_assert(!turner::usable_with<decltype(req), other_proto>);
static_assert(!turner::usable_with<decltype(turner::request<derived_proto, 0x003>), proto>);

template <auto M>
constexpr message_class class_of = turner::__method::class_of<std::remove_cvref_t<decltype(M)>>;

static_assert(class_of<req.request> == message_class::request);
static_assert(class_of<req.success> == message_class::success);
static_assert(class_of<req.error> == message_class::error);
static_assert(class_of<ind.indication> == message_class::indication);
static_assert(class_of<all.indication> == message_class::indication);

} // namespace
