#include <turner/message_reader>
#include <turner/msturn>
#include <turner/stun>
#include <turner/turn>
#include <turner/test>
#include <catch2/catch_template_test_macros.hpp>

// FYI:
// - STUN Fingerprint tests are in turner/stun.test.cpp
// - message_reader::read() tests are in turner/attribute_value_type.test.cpp
// - message_reader::read() + not_read() tests are in turner/attribute_type_list.test.cpp

namespace {

struct msturn //{{{1
{
	using protocol_type = turner::msturn;

	static constexpr uint8_t valid_message[] =
	{
		0x00, 0x03, 0x00, 0x08, // MS-TURN Allocation
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x0f, 0x00, 0x04, // Magic Cookie
		0x72, 0xc6, 0x4b, 0xc6,
	};

	static constexpr auto expected_message_type = turner::msturn::allocate;

	static constexpr turner::msturn::transaction_id_type expected_transaction_id =
	{
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
	};

	static constexpr uint8_t invalid_cookie_message[] =
	{
		0x00, 0x03, 0x00, 0x08, // MS-TURN Allocation
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x0f, 0x00, 0x04, // Magic Cookie (invalid)
		0xff, 0xff, 0xff, 0xff,
	};
};


struct stun //{{{1
{
	using protocol_type = turner::stun;

	static constexpr uint8_t valid_message[] =
	{
		0x00, 0x01, 0x00, 0x08, // STUN Binding
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x80, 0x28, 0x00, 0x04, // Fingerprint
		0x5b, 0x0f, 0xf6, 0xfc,
	};

	static constexpr auto expected_message_type = turner::stun::binding;

	static constexpr turner::stun::transaction_id_type expected_transaction_id =
	{
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
	};

	static constexpr uint8_t invalid_cookie_message[] =
	{
		0x00, 0x01, 0x00, 0x08, // STUN Binding
		0xff, 0xff, 0xff, 0xff, // Magic Cookie (invalid)
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x80, 0x28, 0x00, 0x04, // Fingerprint
		0x5b, 0x0f, 0xf6, 0xfc,
	};
};


struct turn //{{{1
{
	using protocol_type = turner::turn;

	static constexpr uint8_t valid_message[] =
	{
		0x00, 0x03, 0x00, 0x08, // TURN Allocate
		0x21, 0x12, 0xa4, 0x42, // Magic Cookie
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x80, 0x28, 0x00, 0x04, // Fingerprint
		0x2f, 0xbb, 0xf0, 0x0d,
	};

	static constexpr auto expected_message_type = turner::turn::allocate;

	static constexpr turner::turn::transaction_id_type expected_transaction_id =
	{
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
	};

	static constexpr uint8_t invalid_cookie_message[] =
	{
		0x00, 0x03, 0x00, 0x08, // TURN Allocate
		0xff, 0xff, 0xff, 0xff, // Magic Cookie (invalid)
		0x00, 0x01, 0x02, 0x03, // Transaction ID
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x80, 0x28, 0x00, 0x04, // Fingerprint
		0x2f, 0xbb, 0xf0, 0x0d,
	};
};

//}}}1

TEMPLATE_TEST_CASE("message_reader", "",
	msturn,
	stun,
	turn)
{
	using Protocol = typename TestType::protocol_type;

	SECTION("valid")
	{
		auto span = std::as_bytes(std::span{TestType::valid_message});

		auto reader = Protocol::read_message(span);
		REQUIRE(reader);
		CHECK(reader->expect(TestType::expected_message_type));
		CHECK(reader->transaction_id() == TestType::expected_transaction_id);

		auto raw = reader->as_bytes();
		CHECK(raw.data() == span.data());
		CHECK(raw.size_bytes() == span.size_bytes());
	}

	SECTION("iterator")
	{
		auto span = std::as_bytes(std::span{TestType::valid_message});
		auto reader = Protocol::read_message(span);
		REQUIRE(reader);

		// MS-TURN Magic Cookie is intentionally omitted in turner,
		// set it explicitly here
		// For STUN/TURN, use Fingerprint
		constexpr auto expected_type = std::conditional_t<
			std::is_same_v<TestType, msturn>,
			std::integral_constant<uint16_t, 0x000f>,
			std::integral_constant<uint16_t, turner::stun::fingerprint.type>
		>::value;

		SECTION("pre-increment")
		{
			for (auto it = reader->begin();  it != reader->end();  ++it)
			{
				CHECK(it->type == expected_type);
				CHECK(it->data.size() == 4);
				CHECK(it->data.data() > span.data());
				CHECK(it->data.data() < span.data() + span.size());
			}
		}

		SECTION("post-increment")
		{
			auto it = reader->begin(), end = reader->end();
			while (it != end)
			{
				auto entry = *it++;
				CHECK(entry.type == expected_type);
				CHECK(entry.data.size() == 4);
				CHECK(entry.data.data() > span.data());
				CHECK(entry.data.data() < span.data() + span.size());
			}
		}

		SECTION("auto &entry")
		{
			for (auto &entry: *reader)
			{
				CHECK(entry.type == expected_type);
				CHECK(entry.data.size() == 4);
				CHECK(entry.data.data() > span.data());
				CHECK(entry.data.data() < span.data() + span.size());
			}
		}

		SECTION("auto &[type, data]")
		{
			for (auto &[type, data]: *reader)
			{
				CHECK(type == expected_type);
				CHECK(data.size() == 4);
				CHECK(data.data() > span.data());
				CHECK(data.data() < span.data() + span.size());
			}
		}

		SECTION("std::for_each")
		{
			std::for_each(reader->begin(), reader->end(), [&span](auto &entry)
			{
				CHECK(entry.type == expected_type);
				CHECK(entry.data.size() == 4);
				CHECK(entry.data.data() > span.data());
				CHECK(entry.data.data() < span.data() + span.size());
			});
		}
	}

	SECTION("insufficient data")
	{
		uint8_t data[] = { 0x00, 0x01, };
		auto reader = Protocol::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::unexpected_message_length);
	}

	SECTION("unpadded span")
	{
		uint8_t data[29] = { 0x00, 0x01, };
		auto reader = Protocol::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::unexpected_message_length);
	}

	SECTION("invalid magic cookie")
	{
		auto span = std::as_bytes(std::span{TestType::invalid_cookie_message});
		auto reader = Protocol::read_message(span);
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::invalid_magic_cookie);
	}

	SECTION("unexpected message length")
	{
		// claim 16B but actually 8B
		uint8_t data[] =
		{
			0x00, 0x03, 0x00, 0x10,
			0x21, 0x12, 0xa4, 0x42,
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f,
			0x00, 0x0f, 0x00, 0x04,
			0x72, 0xc6, 0x4b, 0xc6,
		};
		auto reader = Protocol::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::unexpected_message_length);
	}

	SECTION("unpadded message length")
	{
		// 9B payload
		uint8_t data[28] =
		{
			0x00, 0x03, 0x00, 0x09,
			0x21, 0x12, 0xa4, 0x42,
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f,
			0x00, 0x0f, 0x00, 0x04,
			0x72, 0xc6, 0x4b, 0xc6,
		};
		auto reader = Protocol::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::unexpected_message_length);
	}

	SECTION("unexpected message type")
	{
		// message type highest bits invalid
		uint8_t data[28] =
		{
			0xff, 0x03, 0x00, 0x08,
			0x21, 0x12, 0xa4, 0x42,
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f,
			0x00, 0x0f, 0x00, 0x04,
			0x72, 0xc6, 0x4b, 0xc6,
		};
		auto reader = Protocol::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::unexpected_message_type);
	}

	SECTION("unexpected attribute length")
	{
		// attribute length is 5B but payload is 4B
		uint8_t data[] =
		{
			0x00, 0x03, 0x00, 0x10,
			0x21, 0x12, 0xa4, 0x42,
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f,

			0x00, 0x0f, 0x00, 0x04,
			0x72, 0xc6, 0x4b, 0xc6,

			0x80, 0x28, 0x00, 0x05,
			0xff, 0xff, 0xff, 0xff,
		};
		auto reader = Protocol::read_message(std::as_bytes(std::span{data}));
		REQUIRE(!reader);
		CHECK(reader.error() == turner::errc::unexpected_attribute_length);
	}

	SECTION("attribute not found")
	{
		auto span = std::as_bytes(std::span{TestType::valid_message});
		auto reader = Protocol::read_message(span).value();
		constexpr auto not_found_attribute = turner::attribute<Protocol, turner::uint32_value_type, 0x80ff>;
		auto value = reader.read(not_found_attribute);
		REQUIRE(!value);
		CHECK(value.error() == turner::errc::attribute_not_found);
	}
}

} // namespace
