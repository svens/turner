#if !defined(_CRT_SECURE_NO_WARNINGS)
	#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstdlib>

#include <turner/test>
#include <pal/version>
#include <catch2/catch_session.hpp>

#if __pal_os_windows && __pal_build_debug

int report_hook (int report_type, char *message, int *return_value)
{
	static const char *level[] = { "WARN", "ERROR", "ASSERT" };
	printf("[%s] %s\n", level[report_type], message);
	*return_value = 0;
	return 1;
}

void set_report_hook ()
{
	_CrtSetReportHook(report_hook);
}

#else

void set_report_hook ()
{ }

#endif

int main (int argc, char *argv[])
{
	set_report_hook();
	return Catch::Session().run(argc, argv);
}

void *operator new (size_t size)
{
	if (turner_test::bad_alloc_once::fail)
	{
		turner_test::bad_alloc_once::fail = false;
		throw std::bad_alloc();
	}
	return std::malloc(size);
}

void operator delete (void *ptr) noexcept
{
	std::free(ptr);
}

void operator delete (void *ptr, size_t) noexcept
{
	std::free(ptr);
}

namespace turner_test {

bool has_ci_environment_variable ()
{
	static const bool has_env = (std::getenv("CI") != nullptr);
	return has_env;
}

} // namespace turner_test
