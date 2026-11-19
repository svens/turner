// Benchmark results (2026-11-19, ninja bench)
//
// benchmark                                   macOS/clang   Linux/GCC   Windows/MSVC
// allocate/parse                                   7.7 ns      7.6 ns         7.5 ns
// allocate/read                                   33.6 ns     30.1 ns        50.0 ns
// authenticated_allocate<sha1>/parse               8.8 ns     10.6 ns        10.7 ns
// authenticated_allocate<sha1>/read               38.6 ns     38.2 ns        44.8 ns
// authenticated_allocate<sha1>/verify            349.8 ns    349.2 ns       411.2 ns
// authenticated_allocate<sha256>/parse             9.2 ns     10.0 ns        10.9 ns
// authenticated_allocate<sha256>/read             38.7 ns     37.9 ns        44.8 ns
// authenticated_allocate<sha256>/verify          565.5 ns    587.5 ns       610.9 ns
// send_request<3, sha1>/parse                     15.7 ns     15.7 ns        15.7 ns
// send_request<3, sha1>/read                      70.2 ns     83.6 ns        80.1 ns
// send_request<3, sha1>/verify                   390.5 ns    392.7 ns       505.1 ns
// send_request<payload_size, sha1>/parse          16.8 ns     15.7 ns        15.5 ns
// send_request<payload_size, sha1>/read           70.6 ns     78.8 ns        81.6 ns
// send_request<payload_size, sha1>/verify       1434.8 ns   1501.6 ns      2307.2 ns
// send_request<payload_size, sha256>/parse        15.4 ns     16.0 ns        15.6 ns
// send_request<payload_size, sha256>/read         70.6 ns     78.9 ns        80.2 ns
// send_request<payload_size, sha256>/verify     3056.3 ns   3052.4 ns      3504.6 ns
// data_indication<3>/parse                         5.9 ns      6.7 ns         6.2 ns
// data_indication<3>/read                         25.3 ns     26.7 ns        28.1 ns
// data_indication<payload_size>/parse              6.4 ns      5.7 ns         6.2 ns
// data_indication<payload_size>/read              25.0 ns     29.2 ns        28.5 ns
// set_active_destination/parse                    13.2 ns     14.4 ns        14.2 ns
// set_active_destination/read                     60.3 ns     67.6 ns        72.9 ns
// set_active_destination/verify                  393.5 ns    400.4 ns       515.6 ns
// tcp_allocate/parse                              12.8 ns     13.7 ns        13.7 ns
// tcp_data/parse                                   2.9 ns      2.0 ns         3.0 ns
// mturn_data<3>/parse                              3.2 ns      2.3 ns         3.3 ns
// mturn_data<payload_size>/parse                   3.1 ns      2.3 ns         3.3 ns

#include <turner/codec/bench.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/msturn.hpp>
#include <turner/codec/test.hpp>
#include <turner/codec/writer.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace
{

using turner::msturn;
using turner_bench::owned;
using turner_bench::payload_size;
using turner_bench::reader_of;
using turner_bench::sha1;
using turner_bench::sha256;
using turner_bench::written;
using turner_test::msturn_id;
using turner_test::peer_address;

struct allocate
{
	static constexpr auto entity = msturn::allocate;
	static constexpr auto attributes = turner::attributes<
		msturn::ms_version,
		msturn::ms_service_quality,
		msturn::lifetime,
		msturn::requested_address_family,
		msturn::username,
		msturn::realm,
		msturn::nonce
	>;

	static std::vector<std::byte> make ()
	{
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, msturn_id}
				.write(msturn::ms_version, msturn::protocol_version::v4)
				.write(
					msturn::ms_service_quality,
					{
						.stream = msturn::stream_type::audio,
						.quality = msturn::delivery::best_effort,
					}
				)
				.write(msturn::username, "alice")
				.finish()
		);
	}
};

template <turner::integrity_algorithm Algorithm>
struct authenticated_allocate
{
	static constexpr auto entity = msturn::allocate;
	static constexpr auto attributes = turner::attributes<
		msturn::ms_version,
		msturn::ms_service_quality,
		msturn::lifetime,
		msturn::requested_address_family,
		msturn::username,
		msturn::realm,
		msturn::nonce
	>;

	static msturn::integrity_key key ()
	{
		return turner_bench::key<msturn, Algorithm>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, msturn_id}
				.write(msturn::ms_version, msturn::protocol_version::v4)
				.write(msturn::username, "user")
				.write(msturn::realm, "realm")
				.write(msturn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

template <size_t Size, turner::integrity_algorithm Algorithm>
struct send_request
{
	static constexpr auto entity = msturn::send_request;
	static constexpr auto attributes = turner::attributes<
		msturn::ms_version,
		msturn::ms_sequence_number,
		msturn::destination_address,
		msturn::data,
		msturn::username,
		msturn::realm,
		msturn::nonce
	>;

	static msturn::integrity_key key ()
	{
		return turner_bench::key<msturn, Algorithm>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		const std::vector<std::byte> payload(Size);
		std::vector<std::byte> buffer(256 + Size);
		return written(
			turner::message_writer<entity>{buffer, msturn_id}
				.write(msturn::ms_version, msturn::protocol_version::v4)
				.write(msturn::ms_sequence_number, {.connection_id = {}, .sequence = 1})
				.write(msturn::destination_address, peer_address)
				.write(msturn::data, payload)
				.write(msturn::username, "user")
				.write(msturn::realm, "realm")
				.write(msturn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

template <size_t Size>
struct data_indication
{
	static constexpr auto entity = msturn::data_indication;
	static constexpr auto attributes = turner::attributes<msturn::remote_address, msturn::data>;

	static std::vector<std::byte> make ()
	{
		constexpr size_t headroom = msturn::max_headroom - msturn::frame_header_size;
		std::vector<std::byte> buffer(headroom + Size + msturn::max_tailroom);
		const auto payload = std::span{buffer}.subspan(headroom, Size);
		return owned(msturn::make_data_indication(buffer, payload, peer_address, msturn_id));
	}
};

struct set_active_destination
{
	static constexpr auto entity = msturn::set_active_destination;
	static constexpr auto attributes = turner::attributes<
		msturn::ms_version,
		msturn::ms_sequence_number,
		msturn::destination_address,
		msturn::username,
		msturn::realm,
		msturn::nonce
	>;

	static msturn::integrity_key key ()
	{
		return turner_bench::key<msturn, sha1>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, msturn_id}
				.write(msturn::ms_version, msturn::protocol_version::v4)
				.write(msturn::ms_sequence_number, {.connection_id = {}, .sequence = 1})
				.write(msturn::destination_address, peer_address)
				.write(msturn::username, "user")
				.write(msturn::realm, "realm")
				.write(msturn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

TEMPLATE_TEST_CASE("codec/msturn", "[!benchmark]",
	allocate,
	authenticated_allocate<sha1>,
	authenticated_allocate<sha256>,
	(send_request<3, sha1>),
	(send_request<payload_size, sha1>),
	(send_request<payload_size, sha256>),
	data_indication<3>,
	data_indication<payload_size>,
	set_active_destination)
{
	auto message = TestType::make();

	BENCHMARK("parse")
	{
		return msturn::parse(message, message.size());
	};

	if constexpr (requires { TestType::attributes; })
	{
		BENCHMARK("read")
		{
			return reader_of<TestType::entity>(message).read(TestType::attributes);
		};
	}

	if constexpr (requires { TestType::key(); })
	{
		auto key = TestType::key();
		const auto reader = reader_of<TestType::entity>(message);
		REQUIRE(reader.verify(key));

		BENCHMARK("verify")
		{
			return reader.verify(key);
		};
	}
}

TEST_CASE("codec/msturn/tcp_allocate", "[!benchmark]")
{
	auto key = turner_bench::key<msturn, sha1>();
	std::vector<std::byte> buffer(256);
	const auto allocate = turner::message_writer<msturn::allocate>{
		std::span{buffer}.subspan(msturn::frame_header_size),
		msturn_id
	}
		.write(msturn::ms_version, msturn::protocol_version::v4)
		.write(msturn::username, "user")
		.write(msturn::realm, "realm")
		.write(msturn::nonce, "nonce")
		.finish(key)
	;
	REQUIRE(allocate);
	auto message = owned(msturn::make_frame(buffer, *allocate, msturn::frame_kind::control));

	BENCHMARK("parse")
	{
		const auto frame = msturn::parse_frame(message, message.size()).value();
		return msturn::parse(std::span{message}.subspan(msturn::frame_header_size), frame.payload().size());
	};
}

TEST_CASE("codec/msturn/tcp_data", "[!benchmark]")
{
	std::vector<std::byte> buffer(msturn::frame_header_size + payload_size);
	const auto payload = std::span{buffer}.subspan(msturn::frame_header_size);
	const auto message = owned(msturn::make_frame(buffer, payload, msturn::frame_kind::data));

	BENCHMARK("parse")
	{
		return msturn::parse_frame(message, message.size());
	};
}

TEMPLATE_TEST_CASE_SIG("codec/msturn/mturn_data", "[!benchmark]", ((size_t Size), Size), 3, payload_size)
{
	constexpr std::array<std::byte, 8> session_id{};
	std::vector<std::byte> buffer(msturn::mturn_header_size + Size);
	const auto payload = std::span{buffer}.subspan(msturn::mturn_header_size);
	const auto message = owned(msturn::make_mturn_data(buffer, payload, session_id));

	BENCHMARK("parse")
	{
		return msturn::parse_mturn_data(message, message.size());
	};
}

} // namespace
