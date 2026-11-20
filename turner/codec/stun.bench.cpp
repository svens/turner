// Benchmark results (2026-11-19, ninja bench)
//
// benchmark                                      macOS/clang   Linux/GCC   Windows/MSVC
// binding_request/parse                               5.6 ns      5.8 ns        10.3 ns
// binding_request_fingerprint/parse                  16.8 ns     21.6 ns        26.1 ns
// authenticated_binding_request<sha1>/parse          11.3 ns     15.1 ns        14.3 ns
// authenticated_binding_request<sha1>/read           15.1 ns     18.3 ns        24.2 ns
// authenticated_binding_request<sha1>/verify        257.9 ns    270.8 ns       317.4 ns
// authenticated_binding_request<sha256>/parse        12.1 ns     15.7 ns        14.8 ns
// authenticated_binding_request<sha256>/read         15.5 ns     20.0 ns        25.1 ns
// authenticated_binding_request<sha256>/verify      444.4 ns    437.0 ns       448.9 ns
// binding_success<peer_address>/parse                40.0 ns     47.5 ns        48.9 ns
// binding_success<peer_address>/read                 66.9 ns     58.3 ns        72.6 ns
// binding_success<peer_address>/verify              269.4 ns    265.5 ns       327.0 ns
// binding_success<ipv6_address>/parse                52.6 ns     50.9 ns        59.9 ns
// binding_success<ipv6_address>/read                 76.5 ns     66.0 ns        88.5 ns
// binding_success<ipv6_address>/verify              341.7 ns    325.1 ns       421.5 ns

#include <turner/codec/bench.hpp>
#include <turner/codec/attribute.hpp>
#include <turner/codec/integrity.hpp>
#include <turner/codec/stun.hpp>
#include <turner/codec/test.hpp>
#include <turner/codec/value.hpp>
#include <turner/codec/writer.hpp>
#include <pal/net/ip/address_v6.hpp>
#include <pal/net/ip/port_type.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

namespace
{

using turner::stun;
using turner_bench::reader_of;
using turner_bench::sha1;
using turner_bench::sha256;
using turner_bench::written;
using turner_test::peer_address;
using turner_test::stun_id;

constexpr turner::transport_address ipv6_address
{
	pal::net::ip::address_v6{
		pal::net::ip::address_v6::bytes_type{
			0x20, 0x01, 0x0d, 0xb8, 0x12, 0x34, 0x56, 0x78, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
		}
	},
	pal::net::ip::port_type{32853},
};

struct binding_request
{
	static constexpr auto entity = stun::binding.request;

	static std::vector<std::byte> make ()
	{
		std::vector<std::byte> buffer(256);
		return written(turner::message_writer<entity>{buffer, stun_id}.finish());
	}
};

struct binding_request_fingerprint
{
	static constexpr auto entity = stun::binding.request;

	static std::vector<std::byte> make ()
	{
		std::vector<std::byte> buffer(256);
		return written(turner::message_writer<entity>{buffer, stun_id}.finish(stun::fingerprint));
	}
};

template <turner::integrity_algorithm Algorithm>
struct authenticated_binding_request
{
	static constexpr auto entity = stun::binding.request;
	static constexpr auto attributes = turner::attributes<stun::username>;

	static stun::integrity_key key ()
	{
		return turner_bench::key<stun, Algorithm>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		return written(turner::message_writer<entity>{buffer, stun_id}.write(stun::username, "user").finish(integrity));
	}
};

template <const turner::transport_address &Mapped>
struct binding_success
{
	static constexpr auto entity = stun::binding.success;
	static constexpr auto attributes = turner::attributes<stun::xor_mapped_address, stun::software>;

	static stun::integrity_key key ()
	{
		return turner_bench::key<stun, sha1>();
	}

	static std::vector<std::byte> make ()
	{
		auto integrity = key();
		std::vector<std::byte> buffer(256);
		const auto request = turner::message_writer<stun::binding.request>{buffer, stun_id}.finish();
		REQUIRE(request);
		return written(
			stun::parse(buffer, request->size()).value().as(stun::binding.request).value().to_success()
				.write(stun::software, "test vector")
				.write(stun::xor_mapped_address, Mapped)
				.finish(integrity, stun::fingerprint)
		);
	}
};

TEMPLATE_TEST_CASE("codec/stun", "[!benchmark]",
	binding_request,
	binding_request_fingerprint,
	authenticated_binding_request<sha1>,
	authenticated_binding_request<sha256>,
	binding_success<peer_address>,
	binding_success<ipv6_address>)
{
	auto message = TestType::make();

	BENCHMARK("parse")
	{
		return stun::parse(message, message.size());
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

} // namespace
