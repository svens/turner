#include <turner/version.hpp>
#include <catch2/catch_test_macros.hpp>
#include <format>

namespace
{

TEST_CASE("version")
{
	auto prefix = std::format("{}.{}.{}", turner::version_major, turner::version_minor, turner::version_patch);
	CHECK(turner::version.starts_with(prefix));
}

} // namespace
