#pragma once

#include <stdexcept>
#include <string>


template <typename OptionHandler>
void parse_command_line (int argc, const char *argv[], OptionHandler option_handler)
{
	for (auto p = argv + 1;  p != argv + argc;  ++p)
	{
		std::string arg{*p};
		if (arg.starts_with("--"))
		{
			auto assign = arg.find('=');
			if (assign != arg.npos)
			{
				std::string option{&arg[2], &arg[assign]};
				std::string argument = arg.substr(assign + 1);
				option_handler(option, argument);
			}
			else
			{
				option_handler(arg, "");
			}
		}
		else
		{
			option_handler("", arg);
		}
	}
}


template <typename Int>
Int parse (const std::string &option, const std::string &argument)
{
	try
	{
		// ignoring range issues
		return static_cast<Int>(std::stoi(argument));
	}
	catch (const std::invalid_argument &)
	{
		throw std::runtime_error(option + ": not valid integer '" + argument + "'");
	}
}
