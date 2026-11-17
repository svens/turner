#include <turner/codec/test.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/turn.hpp>
#include <turner/codec/value.hpp>
#include <pal/result.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{

using namespace std::chrono_literals;

static_assert(turner::stun::username.type == 0x0006);
static_assert(turner::stun::realm.type == 0x0014);
static_assert(turner::stun::nonce.type == 0x0015);
static_assert(turner::stun::software.type == 0x8022);
static_assert(turner::turn::lifetime.type == 0x000d);
static_assert(turner::turn::data.type == 0x0013);
static_assert(turner::turn::requested_address_family.type == 0x0017);
static_assert(turner::turn::requested_transport.type == 0x0019);
static_assert(turner::turn::dont_fragment.type == 0x001a);
static_assert(turner::turn::reservation_token.type == 0x0022);
static_assert(turner::turn::additional_address_family.type == 0x8000);
static_assert(turner::msturn::username.type == 0x0006);
static_assert(turner::msturn::lifetime.type == 0x000d);
static_assert(turner::msturn::bandwidth.type == 0x0010);
static_assert(turner::msturn::data.type == 0x0013);
static_assert(turner::msturn::nonce.type == 0x0014);
static_assert(turner::msturn::realm.type == 0x0015);
static_assert(turner::msturn::requested_address_family.type == 0x0017);

template <auto Attribute, typename Protocol, typename Value>
concept declared_as = std::same_as<typename decltype(Attribute)::protocol_type, Protocol>
	&& std::same_as<typename decltype(Attribute)::value_type, Value>
;

static_assert(declared_as<turner::stun::username, turner::stun, turner::string_value<508>>);
static_assert(declared_as<turner::stun::realm, turner::stun, turner::string_value<763>>);
static_assert(declared_as<turner::stun::nonce, turner::stun, turner::string_value<763>>);
static_assert(declared_as<turner::stun::software, turner::stun, turner::string_value<763>>);
static_assert(declared_as<turner::turn::username, turner::stun, turner::string_value<508>>);
static_assert(declared_as<turner::turn::lifetime, turner::turn, turner::seconds_value>);
static_assert(declared_as<turner::turn::data, turner::turn, turner::bytes_value<>>);
static_assert(declared_as<turner::turn::requested_address_family, turner::turn, turner::address_family_value>);
static_assert(declared_as<turner::turn::requested_transport, turner::turn, turner::transport_protocol_value>);
static_assert(declared_as<turner::turn::dont_fragment, turner::turn, turner::flag_value>);
static_assert(declared_as<turner::turn::reservation_token, turner::turn, turner::bytes_value<8>>);
static_assert(declared_as<turner::turn::additional_address_family, turner::turn, turner::address_family_value>);
static_assert(declared_as<turner::msturn::username, turner::msturn, turner::string_value<>>);
static_assert(declared_as<turner::msturn::lifetime, turner::msturn, turner::seconds_value>);
static_assert(declared_as<turner::msturn::bandwidth, turner::msturn, turner::uint32_value>);
static_assert(declared_as<turner::msturn::data, turner::msturn, turner::bytes_value<>>);
static_assert(declared_as<turner::msturn::nonce, turner::msturn, turner::string_value<128>>);
static_assert(declared_as<turner::msturn::realm, turner::msturn, turner::string_value<128>>);
static_assert(declared_as<turner::msturn::requested_address_family, turner::msturn, turner::address_family_value>);

static_assert(turner::usable_with<decltype(turner::stun::username), turner::turn>);
static_assert(!turner::usable_with<decltype(turner::turn::lifetime), turner::stun>);
static_assert(!turner::usable_with<decltype(turner::msturn::lifetime), turner::turn>);

template <auto Entity, auto Attribute>
concept can_read = requires(const turner::message_reader<Entity> &reader)
{
	reader.read(Attribute);
};

template <auto Entity, auto Attribute>
concept can_read_each = requires(const turner::message_reader<Entity> &reader)
{
	reader.read_each(Attribute);
};

template <auto Entity, auto Attribute, typename Fallback>
concept can_read_or = requires(const turner::message_reader<Entity> &reader, Fallback &&fallback)
{
	reader.read_or(Attribute, std::forward<Fallback>(fallback));
};

template <auto Entity, auto Attribute>
concept can_read_all = can_read<Entity, Attribute>
	&& can_read_each<Entity, Attribute>
	&& can_read_or<Entity, Attribute, const typename decltype(Attribute)::value_type::native_type &>
;

template <auto Entity, auto Attribute>
concept can_read_none = !can_read<Entity, Attribute>
	&& !can_read_each<Entity, Attribute>
	&& !can_read_or<Entity, Attribute, const typename decltype(Attribute)::value_type::native_type &>
;

static_assert(can_read_all<turner::stun::binding.request, turner::stun::username>);
static_assert(can_read_all<turner::turn::allocate, turner::turn::lifetime>);
static_assert(can_read_all<turner::turn::allocate, turner::stun::username>);
static_assert(can_read_all<turner::turn::allocate.success, turner::turn::lifetime>);
static_assert(can_read_all<turner::msturn::allocate, turner::msturn::lifetime>);
static_assert(can_read_none<turner::stun::binding.request, turner::turn::lifetime>);
static_assert(can_read_none<turner::turn::binding.request, turner::turn::lifetime>);
static_assert(can_read_none<turner::turn::allocate, turner::msturn::lifetime>);
static_assert(can_read_none<turner::msturn::allocate, turner::turn::lifetime>);
static_assert(can_read_none<turner::msturn::allocate, turner::stun::username>);

static_assert(can_read_or<turner::turn::allocate, turner::turn::lifetime, std::chrono::seconds>);
static_assert(can_read_or<turner::turn::allocate, turner::turn::lifetime, std::chrono::minutes>);
static_assert(!can_read_or<turner::turn::allocate, turner::turn::lifetime, int>);
static_assert(can_read<turner::turn::allocate, turner::turn::dont_fragment>);
static_assert(can_read_each<turner::turn::allocate, turner::turn::dont_fragment>);
static_assert(!can_read_or<turner::turn::allocate, turner::turn::dont_fragment, bool>);
static_assert(!can_read_or<turner::turn::allocate, turner::turn::dont_fragment, std::true_type>);
static_assert(!can_read_or<turner::turn::allocate, turner::turn::dont_fragment, const std::true_type &>);
static_assert(
	std::same_as<
		decltype(
			std::declval<turner::message_reader<turner::turn::allocate>>().read(turner::turn::dont_fragment)
		),
		pal::result<std::true_type>
	>
);
static_assert(can_read_or<turner::msturn::allocate, turner::msturn::bandwidth, int>);
static_assert(can_read_or<turner::turn::allocate, turner::turn::realm, std::string_view>);
static_assert(can_read_or<turner::turn::allocate, turner::turn::realm, const char *>);
static_assert(can_read_or<turner::turn::allocate, turner::turn::realm, const char (&)[8]>);
static_assert(can_read_or<turner::turn::allocate, turner::turn::realm, const std::string &>);
static_assert(can_read_or<turner::turn::allocate, turner::turn::realm, std::string &>);
static_assert(!can_read_or<turner::turn::allocate, turner::turn::realm, std::string>);
static_assert(can_read_or<turner::turn::send_indication, turner::turn::data, std::span<const std::byte>>);
static_assert(can_read_or<turner::turn::send_indication, turner::turn::data, const std::vector<std::byte> &>);
static_assert(!can_read_or<turner::turn::send_indication, turner::turn::data, std::vector<std::byte>>);
static_assert(!can_read_or<turner::turn::allocate, turner::turn::realm, std::vector<int>>);

using allocate_reader = turner::message_reader<turner::turn::allocate>;
using lifetime_range = decltype(std::declval<allocate_reader>().read_each(turner::turn::lifetime));
using wire_range = decltype(std::declval<allocate_reader>().wire_attributes());

static_assert(
	std::same_as<
		decltype(std::declval<allocate_reader>().read(turner::turn::lifetime)),
		pal::result<std::chrono::seconds>
	>
);
static_assert(
	std::same_as<
		decltype(std::declval<allocate_reader>().read_or(turner::turn::lifetime, 1s)),
		pal::result<std::chrono::seconds>
	>
);

static_assert(
	std::same_as<allocate_reader::wire_attribute_iterator, turner::wire_attribute_iterator<turner::turn>>
);
static_assert(
	std::same_as<
		turner::message_reader<turner::turn::binding.request>::wire_attribute_iterator,
		turner::wire_attribute_iterator<turner::stun>
	>
);
static_assert(
	std::same_as<
		turner::message_reader<turner::msturn::allocate>::wire_attribute_iterator,
		turner::wire_attribute_iterator<turner::msturn>
	>
);
static_assert(
	std::same_as<
		std::ranges::iterator_t<lifetime_range>,
		turner::attribute_value_iterator<turner::turn, turner::seconds_value, 0x000d>
	>
);
static_assert(std::forward_iterator<allocate_reader::wire_attribute_iterator>);
static_assert(std::is_nothrow_default_constructible_v<allocate_reader::wire_attribute_iterator>);
static_assert(std::ranges::forward_range<wire_range>);
static_assert(std::ranges::common_range<wire_range>);
static_assert(std::ranges::borrowed_range<wire_range>);
static_assert(std::ranges::view<wire_range>);
static_assert(std::same_as<std::ranges::range_value_t<wire_range>, turner::wire_attribute>);

static_assert(std::forward_iterator<std::ranges::iterator_t<lifetime_range>>);
static_assert(std::is_nothrow_default_constructible_v<std::ranges::iterator_t<lifetime_range>>);
static_assert(std::ranges::forward_range<lifetime_range>);
static_assert(std::ranges::forward_range<const lifetime_range>);
static_assert(std::ranges::common_range<lifetime_range>);
static_assert(std::ranges::view<lifetime_range>);
static_assert(std::same_as<std::ranges::range_value_t<lifetime_range>, pal::result<std::chrono::seconds>>);
static_assert(
	std::same_as<lifetime_range, allocate_reader::attribute_value_range<turner::turn::lifetime>>
);
static_assert(
	std::same_as<
		decltype(std::declval<allocate_reader>().read_each(turner::turn::username)),
		allocate_reader::attribute_value_range<turner::stun::username>
	>
);
static_assert(
	std::same_as<
		allocate_reader::attribute_value_iterator<turner::stun::username>,
		turner::attribute_value_iterator<turner::turn, turner::string_value<508>, 0x0006>
	>
);

struct message_size_value
{
	using native_type = size_t;
	static constexpr size_t min_size = 0;
	static constexpr size_t max_size = 0xffff;

	static size_t size (const native_type &) noexcept
	{
		return 0;
	}

	static pal::result<native_type> read (std::span<const std::byte> message, std::span<const std::byte>) noexcept
	{
		return message.size();
	}

	static void write (std::span<const std::byte>, std::span<std::byte>, const native_type &) noexcept
	{
	}
};

constexpr uint16_t other_type = 0x8080;

struct stun
{
	using protocol_type = turner::stun;
	static constexpr auto message = turner::stun::binding.indication;
	static constexpr auto &without_attributes = turner_test::stun_message::binding_indication;
	static constexpr auto attribute = turner::stun::username;
	static constexpr auto absent = turner::stun::software;
};

struct turn
{
	using protocol_type = turner::turn;
	static constexpr auto message = turner::turn::refresh.success;
	static constexpr auto &without_attributes = turner_test::turn_message::refresh_success;
	static constexpr auto attribute = turner::turn::username;
	static constexpr auto absent = turner::turn::software;
};

struct msturn
{
	using protocol_type = turner::msturn;
	static constexpr auto message = turner::msturn::set_active_destination.success;
	static constexpr auto &without_attributes = turner_test::msturn_message::set_active_destination_success;
	static constexpr auto attribute = turner::msturn::nonce;
	static constexpr auto absent = turner::msturn::realm;
};

constexpr std::byte garbage{0xa5};
constexpr size_t trailing_size = 7;

std::vector<std::byte> make_buffer (std::span<const std::byte> bytes, size_t trailing = trailing_size)
{
	std::vector<std::byte> buffer{bytes.begin(), bytes.end()};
	buffer.resize(buffer.size() + trailing, garbage);
	return buffer;
}

template <typename Protocol>
void append (std::vector<std::byte> &message, uint16_t type, std::string_view value)
{
	message.push_back(static_cast<std::byte>(type >> 8));
	message.push_back(static_cast<std::byte>(type & 0xff));
	message.push_back(static_cast<std::byte>(value.size() >> 8));
	message.push_back(static_cast<std::byte>(value.size() & 0xff));
	const auto bytes = std::as_bytes(std::span{value});
	message.insert(message.end(), bytes.begin(), bytes.end());
	while ((message.size() - Protocol::header_size) % Protocol::attribute_alignment != 0)
	{
		message.push_back(garbage);
	}

	const auto length = message.size() - Protocol::header_size;
	message[2] = static_cast<std::byte>(length >> 8);
	message[3] = static_cast<std::byte>(length & 0xff);
}

template <typename Protocol, auto Entity>
turner::message_reader<Entity> read_as (std::vector<std::byte> &buffer)
{
	const auto message = Protocol::parse(buffer, buffer.size());
	REQUIRE(message);
	const auto reader = message->as(Entity);
	REQUIRE(reader);
	return *reader;
}

pal::unexpected failed (turner::errc error) noexcept
{
	return turner::make_unexpected(error);
}

using type_and_size = std::pair<uint16_t, size_t>;

std::vector<type_and_size> types_and_sizes (std::ranges::input_range auto &&attributes)
{
	std::vector<type_and_size> result;
	for (const auto &[type, value]: attributes)
	{
		result.emplace_back(type, value.size());
	}
	return result;
}

bool equal (std::span<const std::byte> bytes, std::string_view text) noexcept
{
	return std::ranges::equal(bytes, std::as_bytes(std::span{text}));
}

size_t count (turner::message_reader<turner::turn::refresh>::attribute_value_range<turner::turn::lifetime> lifetimes)
{
	return static_cast<size_t>(std::ranges::distance(lifetimes));
}

TEMPLATE_TEST_CASE("attribute/reader", "", stun, turn, msturn)
{
	using Protocol = typename TestType::protocol_type;
	constexpr auto attribute = TestType::attribute;
	constexpr auto absent = TestType::absent;
	constexpr auto max_size = decltype(attribute)::value_type::max_size;

	std::vector<std::byte> buffer{TestType::without_attributes.begin(), TestType::without_attributes.end()};
	const auto attributes_offset = buffer.size();

	SECTION("repeated attribute")
	{
		append<Protocol>(buffer, attribute.type, "a");
		append<Protocol>(buffer, other_type, "1234");
		append<Protocol>(buffer, attribute.type, "bcd");
		append<Protocol>(buffer, attribute.type, "");
		const auto message_size = buffer.size();

		append<Protocol>(buffer, attribute.type, "after message");
		buffer[2] = static_cast<std::byte>((message_size - Protocol::header_size) >> 8);
		buffer[3] = static_cast<std::byte>((message_size - Protocol::header_size) & 0xff);
		buffer.resize(buffer.size() + trailing_size, garbage);

		const auto reader = read_as<Protocol, TestType::message>(buffer);

		SECTION("read")
		{
			const auto value = reader.read(attribute);
			REQUIRE(value);
			CHECK(*value == "a");
			CHECK(static_cast<const void *>(value->data()) == buffer.data() + attributes_offset + 4);
			CHECK(reader.read(absent) == failed(turner::errc::attribute_not_found));
		}

		SECTION("read_or")
		{
			CHECK(reader.read_or(attribute, "fallback") == "a");
			CHECK(reader.read_or(absent, "fallback") == "fallback");
		}

		SECTION("read_each")
		{
			const auto values = reader.read_each(attribute);
			CHECK_FALSE(values.empty());
			CHECK(std::ranges::distance(values) == 3);

			std::vector<std::string_view> valid;
			for (const auto &value: values)
			{
				REQUIRE(value);
				valid.push_back(*value);
			}
			CHECK(valid == std::vector<std::string_view>{"a", "bcd", ""});

			const std::vector results(values.begin(), values.end());
			CHECK(results.size() == 3);

			CHECK(reader.read_each(absent).empty());
		}

		SECTION("value iterator")
		{
			const auto values = reader.read_each(attribute);
			using iterator = std::ranges::iterator_t<decltype(values)>;

			auto it = values.begin();
			const auto first = it;
			REQUIRE(it != values.end());
			CHECK(it == first);
			CHECK(it->has_value());
			CHECK(*it == "a");

			const auto previous = it++;
			CHECK(previous == first);
			CHECK(it != first);
			REQUIRE(it != values.end());
			CHECK(*previous == "a");
			CHECK(*it == "bcd");

			++it;
			REQUIRE(it != values.end());
			CHECK(*it == "");

			++it;
			CHECK(it == values.end());
			CHECK(iterator{} == iterator{});
		}

		SECTION("wire_attributes")
		{
			const std::vector<type_and_size> expected =
			{
				{attribute.type, 1}, {other_type, 4}, {attribute.type, 3}, {attribute.type, 0},
			};
			CHECK(types_and_sizes(reader.wire_attributes()) == expected);
		}

		SECTION("wire iterator")
		{
			const auto attributes = reader.wire_attributes();
			using iterator = std::ranges::iterator_t<decltype(attributes)>;

			auto it = attributes.begin();
			const auto first = it;
			REQUIRE(it != attributes.end());
			CHECK(it->type == attribute.type);
			CHECK(equal(it->value, "a"));

			const auto previous = it++;
			CHECK(previous == first);
			CHECK(it != first);
			REQUIRE(it != attributes.end());
			CHECK(previous->type == attribute.type);
			CHECK(it->type == other_type);
			CHECK(equal(it->value, "1234"));

			CHECK(std::ranges::distance(attributes) == 4);
			CHECK(std::distance(attributes.begin(), attributes.end()) == 4);
			CHECK(iterator{} == iterator{});
		}

		SECTION("message passed to value type")
		{
			constexpr auto size_of_message = turner::attribute<Protocol, message_size_value, other_type>;
			CHECK(reader.read(size_of_message) == message_size);
			CHECK(reader.read_or(size_of_message, 0) == message_size);
			for (const auto &size: reader.read_each(size_of_message))
			{
				CHECK(size == message_size);
			}
		}
	}

	SECTION("no attributes")
	{
		buffer.resize(buffer.size() + trailing_size, garbage);
		const auto reader = read_as<Protocol, TestType::message>(buffer);

		CHECK(reader.wire_attributes().empty());
		CHECK(reader.read(attribute) == failed(turner::errc::attribute_not_found));
		CHECK(reader.read_or(attribute, "fallback") == "fallback");
		CHECK(reader.read_each(attribute).empty());
	}

	SECTION("value at size limit")
	{
		append<Protocol>(buffer, attribute.type, std::string(max_size, 'x'));
		buffer.resize(buffer.size() + trailing_size, garbage);
		const auto reader = read_as<Protocol, TestType::message>(buffer);

		const auto value = reader.read(attribute);
		REQUIRE(value);
		CHECK(value->size() == max_size);
	}

	SECTION("malformed first occurrence")
	{
		append<Protocol>(buffer, attribute.type, std::string(max_size + 1, 'x'));
		append<Protocol>(buffer, attribute.type, "ok");
		buffer.resize(buffer.size() + trailing_size, garbage);
		const auto reader = read_as<Protocol, TestType::message>(buffer);

		CHECK(reader.read(attribute) == failed(turner::errc::unexpected_attribute_length));
		CHECK(reader.read_or(attribute, "fallback") == failed(turner::errc::unexpected_attribute_length));

		const auto values = reader.read_each(attribute);
		const std::vector results(values.begin(), values.end());
		REQUIRE(results.size() == 2);
		CHECK(results[0] == failed(turner::errc::unexpected_attribute_length));
		CHECK(results[1] == "ok");
	}
}

TEST_CASE("attribute/values")
{
	SECTION("stun: rfc5769 request")
	{
		auto buffer = make_buffer(turner_test::rfc5769::request);
		const auto request = read_as<turner::stun, turner::stun::binding.request>(buffer);

		const auto software = request.read(turner::stun::software);
		REQUIRE(software);
		CHECK(*software == "STUN test client");
		CHECK(static_cast<const void *>(software->data()) == buffer.data() + 24);

		CHECK(request.read(turner::stun::username) == "evtj:h6vY");
		CHECK(request.read(turner::stun::realm) == failed(turner::errc::attribute_not_found));
		CHECK(request.read(turner::stun::nonce) == failed(turner::errc::attribute_not_found));
		CHECK(request.read_or(turner::stun::realm, "fallback") == "fallback");
		CHECK(request.read_or(turner::stun::username, "fallback") == "evtj:h6vY");

		const std::vector<type_and_size> expected =
		{
			{0x8022, 16}, {0x0024, 4}, {0x8029, 8}, {0x0006, 9}, {0x0008, 20}, {0x8028, 4},
		};
		CHECK(types_and_sizes(request.wire_attributes()) == expected);
	}

	SECTION("stun: rfc5769 long term request")
	{
		auto buffer = make_buffer(turner_test::rfc5769::long_term_request);
		const auto request = read_as<turner::stun, turner::stun::binding.request>(buffer);

		CHECK(
			request.read(turner::stun::username)
			== "\xe3\x83\x9e\xe3\x83\x88\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9"
		);
		CHECK(request.read(turner::stun::nonce) == "f//499k954d6OL34oL9FSTvy64sA");
		CHECK(request.read(turner::stun::realm) == "example.org");
	}

	SECTION("turn: allocate request")
	{
		auto buffer = make_buffer(turner_test::turn_message::allocate_request);
		const auto request = read_as<turner::turn, turner::turn::allocate>(buffer);

		CHECK(request.read(turner::turn::requested_transport) == turner::transport_protocol::udp);
		CHECK(request.read(turner::turn::lifetime) == failed(turner::errc::attribute_not_found));
		CHECK(request.read_or(turner::turn::lifetime, 600s) == 600s);
		CHECK(request.read_or(turner::turn::lifetime, 10min) == 600s);
		CHECK(request.read(turner::turn::dont_fragment) == failed(turner::errc::attribute_not_found));
		CHECK_FALSE(request.read(turner::turn::dont_fragment));
		CHECK_FALSE(request.read(turner::turn::dont_fragment) == true);
		CHECK(request.read(turner::turn::username) == failed(turner::errc::attribute_not_found));
		CHECK(types_and_sizes(request.wire_attributes()) == std::vector<type_and_size>{{0x0019, 4}});
	}

	SECTION("turn: flag")
	{
		auto buffer = make_buffer(turner_test::turn_message::allocate_request_dont_fragment);
		const auto request = read_as<turner::turn, turner::turn::allocate>(buffer);

		const auto dont_fragment = request.read(turner::turn::dont_fragment);
		REQUIRE(dont_fragment);
		CHECK(*dont_fragment);
		CHECK(dont_fragment == true);
		CHECK_FALSE(dont_fragment == false);
		CHECK(std::ranges::distance(request.read_each(turner::turn::dont_fragment)) == 1);
		CHECK(request.read(turner::turn::requested_transport) == turner::transport_protocol::udp);
		CHECK(
			types_and_sizes(request.wire_attributes())
			== std::vector<type_and_size>{{0x001a, 0}, {0x0019, 4}}
		);
	}

	SECTION("turn: invalid enum value")
	{
		auto buffer = make_buffer(turner_test::turn_message::allocate_request);
		buffer[turner::turn::header_size + 4] = std::byte{0x07};
		const auto request = read_as<turner::turn, turner::turn::allocate>(buffer);

		CHECK(
			request.read(turner::turn::requested_transport)
			== failed(turner::errc::unexpected_attribute_value)
		);
		CHECK(
			request.read_or(turner::turn::requested_transport, turner::transport_protocol::udp)
			== failed(turner::errc::unexpected_attribute_value)
		);
	}

	SECTION("turn: seconds")
	{
		auto buffer = make_buffer(turner_test::turn_message::allocate_success);
		const auto success = read_as<turner::turn, turner::turn::allocate.success>(buffer);
		CHECK(success.read(turner::turn::lifetime) == 600s);

		auto refresh_buffer = make_buffer(turner_test::turn_message::refresh_request);
		const auto refresh = read_as<turner::turn, turner::turn::refresh>(refresh_buffer);
		CHECK(refresh.read(turner::turn::lifetime) == 0s);
	}

	SECTION("turn: bytes")
	{
		auto buffer = make_buffer(turner_test::turn_message::send_indication);
		const auto indication = read_as<turner::turn, turner::turn::send_indication>(buffer);

		const auto data = indication.read(turner::turn::data);
		REQUIRE(data);
		CHECK(equal(*data, "hello"));
		CHECK(data->data() == buffer.data() + 36);

		CHECK(indication.read(turner::turn::reservation_token) == failed(turner::errc::attribute_not_found));
	}

	SECTION("turn: stun attributes")
	{
		auto buffer = make_buffer(turner_test::turn_message::allocate_error);
		const auto error = read_as<turner::turn, turner::turn::allocate.error>(buffer);

		CHECK(error.read(turner::turn::realm) == "example.org");
		CHECK(error.read(turner::stun::nonce) == "nonce");
	}

	SECTION("turn: repeated fixed-size attribute")
	{
		auto buffer = make_buffer(turner_test::turn_message::refresh_request_repeated_lifetime);
		const auto request = read_as<turner::turn, turner::turn::refresh>(buffer);

		CHECK(request.read(turner::turn::lifetime) == 10s);
		CHECK(request.read_or(turner::turn::lifetime, 0s) == 10s);

		const auto lifetimes = request.read_each(turner::turn::lifetime);
		const std::vector results(lifetimes.begin(), lifetimes.end());
		REQUIRE(results.size() == 3);
		CHECK(results[0] == 10s);
		CHECK(results[1] == 20s);
		CHECK(results[2] == failed(turner::errc::unexpected_attribute_length));
		CHECK(count(lifetimes) == 3);

		CHECK(std::ranges::distance(request.read_each(turner::turn::data)) == 1);
		CHECK(std::ranges::distance(request.read_each(turner::turn::software)) == 1);
		CHECK(request.read_each(turner::turn::realm).empty());
	}

	SECTION("turn: malformed fixed-size attribute first")
	{
		auto buffer = make_buffer(turner_test::turn_message::refresh_request_malformed_lifetime_first);
		const auto request = read_as<turner::turn, turner::turn::refresh>(buffer);

		CHECK(request.read(turner::turn::lifetime) == failed(turner::errc::unexpected_attribute_length));
		CHECK(request.read_or(turner::turn::lifetime, 1s) == failed(turner::errc::unexpected_attribute_length));
		CHECK(count(request.read_each(turner::turn::lifetime)) == 2);
	}

	SECTION("msturn: allocate request")
	{
		auto buffer = make_buffer(turner_test::msturn_message::allocate_request);
		const auto request = read_as<turner::msturn, turner::msturn::allocate>(buffer);

		CHECK(request.read(turner::msturn::username) == "alice");
		CHECK(request.read(turner::msturn::lifetime) == failed(turner::errc::attribute_not_found));
		CHECK(request.read(turner::msturn::bandwidth) == failed(turner::errc::attribute_not_found));
		CHECK(request.read_or(turner::msturn::bandwidth, 100) == 100U);
		CHECK(
			types_and_sizes(request.wire_attributes())
			== std::vector<type_and_size>{{0x8008, 4}, {0x8055, 4}, {0x0006, 5}}
		);
	}

	SECTION("msturn: allocate success")
	{
		auto buffer = make_buffer(turner_test::msturn_message::allocate_success);
		const auto success = read_as<turner::msturn, turner::msturn::allocate.success>(buffer);

		CHECK(success.read(turner::msturn::lifetime) == 600s);
		CHECK(success.read(turner::msturn::username) == "alice");
	}

	SECTION("msturn: allocate error")
	{
		auto buffer = make_buffer(turner_test::msturn_message::allocate_error);
		const auto error = read_as<turner::msturn, turner::msturn::allocate.error>(buffer);

		CHECK(error.read(turner::msturn::nonce) == "nonce");
		CHECK(error.read(turner::msturn::realm) == "example.org");
	}

	SECTION("msturn: empty values")
	{
		auto buffer = make_buffer(turner_test::msturn_message::allocate_request_empty_attributes);
		const auto request = read_as<turner::msturn, turner::msturn::allocate>(buffer);

		CHECK(request.read(turner::msturn::username) == "");
		CHECK(request.read(turner::msturn::realm) == "");
		CHECK(
			types_and_sizes(request.wire_attributes())
			== std::vector<type_and_size>{{0x0006, 0}, {0x8008, 4}, {0x0015, 0}}
		);
	}

	SECTION("msturn: bytes")
	{
		auto buffer = make_buffer(turner_test::msturn_message::send_request);
		const auto request = read_as<turner::msturn, turner::msturn::send_request>(buffer);

		const auto data = request.read(turner::msturn::data);
		REQUIRE(data);
		CHECK(equal(*data, "abc"));
		CHECK(request.read(turner::msturn::username) == "alice");
	}
}

} // namespace
