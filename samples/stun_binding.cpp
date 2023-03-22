// stun_binding: run STUN Binding request against specified server and print
// returned mapped address to std::cout

#include <samples/command_line.hpp>
#include <iostream>


class config
{
public:

	config (int argc, const char *argv[])
	{
		(void)argc;
		(void)argv;
	}

	void print () const
	{ }
};


int run (const config &config)
{
	config.print();
	return EXIT_FAILURE;
}


int main (int argc, const char *argv[])
{
	try
	{
		return run(config{argc, argv});
	}
	catch (const std::exception &e)
	{
		std::cerr << argv[0] << ": " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
