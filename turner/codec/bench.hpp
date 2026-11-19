#pragma once

/**
 * \file turner/codec/bench.hpp
 * Benchmark helpers
 * \internal
 */

#include <turner/codec/integrity.hpp>
#include <turner/codec/reader.hpp>
#include <turner/codec/test.hpp>
#include <pal/result.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <span>
#include <vector>

namespace turner_bench
{

inline constexpr size_t payload_size = 1200;

inline constexpr auto sha1 = turner::integrity_algorithm::sha1;
inline constexpr auto sha256 = turner::integrity_algorithm::sha256;

inline std::vector<std::byte> owned (std::span<const std::byte> message)
{
	return {message.begin(), message.end()};
}

inline std::vector<std::byte> written (const pal::result<std::span<const std::byte>> &message)
{
	REQUIRE(message);
	return owned(*message);
}

template <typename Protocol, turner::integrity_algorithm Algorithm>
typename Protocol::integrity_key key ()
{
	if constexpr (Algorithm == sha1)
	{
		return turner_test::sha1_key<Protocol>();
	}
	else
	{
		return turner_test::sha256_key<Protocol>();
	}
}

template <auto Entity>
turner::message_reader<Entity> reader_of (std::vector<std::byte> &message)
{
	using protocol = typename turner::message_reader<Entity>::protocol_type;
	return protocol::parse(message, message.size()).value().as(Entity).value();
}

} // namespace turner_bench
