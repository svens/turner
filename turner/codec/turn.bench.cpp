// Benchmark results (2026-11-19, ninja bench)
//
// benchmark                               macOS/clang   Linux/GCC   Windows/MSVC
// allocate/parse                               8.3 ns     11.8 ns        12.5 ns
// allocate/read                               37.3 ns     24.5 ns        36.8 ns
// authenticated_allocate<sha1>/parse          21.2 ns     34.4 ns        26.4 ns
// authenticated_allocate<sha1>/read           58.4 ns     61.7 ns        59.6 ns
// authenticated_allocate<sha1>/verify        347.8 ns    344.7 ns       425.3 ns
// authenticated_allocate<sha256>/parse        21.8 ns     32.8 ns        25.9 ns
// authenticated_allocate<sha256>/read         58.9 ns     61.3 ns        60.3 ns
// authenticated_allocate<sha256>/verify      581.5 ns    578.5 ns       612.2 ns
// allocate_success/parse                      15.4 ns     26.3 ns        19.9 ns
// allocate_success/read                       46.0 ns     62.0 ns        64.6 ns
// allocate_success/verify                    269.3 ns    261.5 ns       316.3 ns
// refresh/parse                               18.8 ns     30.1 ns        22.8 ns
// refresh/read                                39.3 ns     48.1 ns        43.3 ns
// refresh/verify                             328.5 ns    325.5 ns       422.1 ns
// send_indication<5>/parse                    10.7 ns     16.6 ns        14.5 ns
// send_indication<5>/read                     32.1 ns     39.1 ns        40.4 ns
// send_indication<payload_size>/parse         11.1 ns     16.6 ns        14.6 ns
// send_indication<payload_size>/read          31.4 ns     31.9 ns        40.3 ns
// data_indication<5>/parse                    10.3 ns     16.2 ns        14.4 ns
// data_indication<5>/read                     32.0 ns     32.7 ns        40.3 ns
// data_indication<payload_size>/parse         10.3 ns     16.4 ns        14.6 ns
// data_indication<payload_size>/read          32.3 ns     32.8 ns        43.8 ns
// create_permission/parse                     18.5 ns     29.7 ns        22.8 ns
// create_permission/read                      47.2 ns     52.6 ns        57.4 ns
// create_permission/verify                   345.0 ns    351.6 ns       420.9 ns
// channel_bind/parse                          21.5 ns     34.5 ns        25.3 ns
// channel_bind/read                           50.9 ns     70.6 ns        68.7 ns
// channel_bind/verify                        351.3 ns    366.0 ns       415.9 ns
// channel_data<5>/parse                        3.1 ns      2.7 ns         3.4 ns
// channel_data<payload_size>/parse             3.1 ns      2.7 ns         3.4 ns

#include <turner/codec/bench.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/protocol.hpp>
#include <turner/codec/test.hpp>
#include <turner/codec/turn.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/writer.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstddef>
#include <span>
#include <vector>

namespace
{

using namespace std::chrono_literals;
using turner::turn;
using turner_bench::owned;
using turner_bench::payload_size;
using turner_bench::reader_of;
using turner_bench::sha1;
using turner_bench::sha256;
using turner_bench::written;
using turner_test::peer_address;
using turner_test::stun_id;

struct allocate
{
	static constexpr auto entity = turn::allocate;
	static constexpr auto attributes = turner::attributes<
		turn::requested_transport,
		turn::requested_address_family,
		turn::lifetime,
		turn::dont_fragment,
		turn::username,
		turn::realm,
		turn::nonce
	>;

	static std::vector<std::byte> make ()
	{
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, stun_id}
				.write(turn::requested_transport, turner::transport_protocol::udp)
				.finish()
		);
	}
};

template <turner::integrity_algorithm Algorithm>
struct authenticated_allocate
{
	static constexpr auto entity = turn::allocate;
	static constexpr auto attributes = turner::attributes<
		turn::requested_transport,
		turn::requested_address_family,
		turn::lifetime,
		turn::dont_fragment,
		turn::username,
		turn::realm,
		turn::nonce
	>;

	static turn::integrity_key key ()
	{
		return turner_bench::key<turn, Algorithm>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, stun_id}
				.write(turn::requested_transport, turner::transport_protocol::udp)
				.write(turn::lifetime, 600s)
				.write(turn::username, "user")
				.write(turn::realm, "realm")
				.write(turn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

struct allocate_success
{
	static constexpr auto entity = turn::allocate.success;
	static constexpr auto attributes = turner::attributes<
		turn::xor_relayed_address,
		turn::lifetime,
		turn::xor_mapped_address
	>;

	static turn::integrity_key key ()
	{
		return turner_bench::key<turn, sha1>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		const auto request = turner::message_writer<turn::allocate>{buffer, stun_id}
			.write(turn::requested_transport, turner::transport_protocol::udp)
			.finish()
		;
		REQUIRE(request);
		return written(
			turn::parse(buffer, request->size()).value().as(turn::allocate).value().to_success()
				.write(turn::xor_relayed_address, peer_address)
				.write(turn::lifetime, 600s)
				.write(turn::xor_mapped_address, peer_address)
				.finish(integrity)
		);
	}
};

struct refresh
{
	static constexpr auto entity = turn::refresh;
	static constexpr auto attributes = turner::attributes<turn::lifetime, turn::username, turn::realm, turn::nonce>;

	static turn::integrity_key key ()
	{
		return turner_bench::key<turn, sha1>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, stun_id}
				.write(turn::lifetime, 600s)
				.write(turn::username, "user")
				.write(turn::realm, "realm")
				.write(turn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

template <size_t Size>
struct send_indication
{
	static constexpr auto entity = turn::send_indication;
	static constexpr auto attributes = turner::attributes<turn::xor_peer_address, turn::data>;

	static std::vector<std::byte> make ()
	{
		const std::vector<std::byte> payload(Size);
		std::vector<std::byte> buffer(256 + Size);
		return written(
			turner::message_writer<entity>{buffer, stun_id}
				.write(turn::xor_peer_address, peer_address)
				.write(turn::data, payload)
				.finish()
		);
	}
};

template <size_t Size>
struct data_indication
{
	static constexpr auto entity = turn::data_indication;
	static constexpr auto attributes = turner::attributes<turn::xor_peer_address, turn::data>;

	static std::vector<std::byte> make ()
	{
		std::vector<std::byte> buffer(turn::max_headroom + Size + turn::max_tailroom);
		const auto payload = std::span{buffer}.subspan(turn::max_headroom, Size);
		return owned(turn::make_data_indication(buffer, payload, peer_address, stun_id));
	}
};

struct create_permission
{
	static constexpr auto entity = turn::create_permission;
	static constexpr auto attributes = turner::attributes<
		turn::xor_peer_address,
		turn::username,
		turn::realm,
		turn::nonce
	>;

	static turn::integrity_key key ()
	{
		return turner_bench::key<turn, sha1>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, stun_id}
				.write(turn::xor_peer_address, peer_address)
				.write(turn::username, "user")
				.write(turn::realm, "realm")
				.write(turn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

struct channel_bind
{
	static constexpr auto entity = turn::channel_bind;
	static constexpr auto attributes = turner::attributes<
		turn::channel_number,
		turn::xor_peer_address,
		turn::username,
		turn::realm,
		turn::nonce
	>;

	static turn::integrity_key key ()
	{
		return turner_bench::key<turn, sha1>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(
			turner::message_writer<entity>{buffer, stun_id}
				.write(turn::channel_number, 0x4000)
				.write(turn::xor_peer_address, peer_address)
				.write(turn::username, "user")
				.write(turn::realm, "realm")
				.write(turn::nonce, "nonce")
				.finish(integrity)
		);
	}
};

TEMPLATE_TEST_CASE("codec/turn", "[!benchmark]",
	allocate,
	authenticated_allocate<sha1>,
	authenticated_allocate<sha256>,
	allocate_success,
	refresh,
	send_indication<5>,
	send_indication<payload_size>,
	data_indication<5>,
	data_indication<payload_size>,
	create_permission,
	channel_bind)
{
	auto message = TestType::make();

	BENCHMARK("parse")
	{
		return turn::parse(message, message.size());
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

TEMPLATE_TEST_CASE_SIG("codec/turn/channel_data", "[!benchmark]", ((size_t Size), Size), 5, payload_size)
{
	std::vector<std::byte> buffer(4 + Size + turn::max_tailroom);
	const auto payload = std::span{buffer}.subspan(4, Size);
	const auto message = owned(turn::make_channel_data(buffer, payload, 0x4000));

	BENCHMARK("parse")
	{
		return turn::parse_channel_data(message, message.size(), turner::transport::datagram);
	};
}

} // namespace
