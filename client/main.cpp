#include <client/main.hpp>
#include <turner/config.hpp>
#include <iostream>
#include <iomanip>
#include <map>


namespace {


using command_fn = int(*)(int argc, const char *[]);

const std::map<std::string, std::pair<command_fn, std::string>> commands =
{
  #define CMD(entry,help) { #entry, { &entry, help }}
  CMD(binding, "Execute STUN Binding request against specified server"),
  CMD(help, "Print this helpful screen"),
  CMD(version, "Print version and exit"),
  #undef CMD
};


int usage (const std::string &message = "")
{
  int exit_code = EXIT_SUCCESS;

  if (!message.empty())
  {
    std::cerr << argv0 << ": " << message << "\n\n";
    exit_code = EXIT_FAILURE;
  }

  std::cout
    << "usage:\n"
    << "  " << argv0 << " command [arguments]\n\n"
    << "command:\n";

  for (auto &command: commands)
  {
    std::cout
      << "  " << std::setw(12) << std::left << command.first
      << '\t' << command.second.second
      << '\n';
  }

  std::cout << "\nRun '" << argv0
    << " command --help' for command-specific help"
    << '\n';

  return exit_code;
}


inline std::string basename (std::string path)
{
  auto last_segment = path.find_last_of("\\/");
  if (last_segment != std::string::npos)
  {
    return path.substr(last_segment + 1);
  }
  return path;
}


} // namespace


int help (int, const char *[])
{
  return usage();
}


int version (int, const char *[])
{
  std::cout << turner::version << " (built " __DATE__ " " __TIME__ ")\n";
  return EXIT_SUCCESS;
}


std::string argv0 = "";


int main (int argc, const char *argv[])
{
  argv0 = basename(argv[0]);

  if (argc < 2)
  {
    return usage("no command specified");
  }

  std::string command_name = argv[1];
  auto command = commands.find(command_name);
  if (command != commands.end())
  {
    try
    {
      return command->second.first(argc - 1, argv + 1);
    }
    catch (const std::exception &e)
    {
      std::cerr << argv0 << ": " << e.what() << '\n';
      return EXIT_FAILURE;
    }
  }

  return usage("unknown command: " + command_name);
}
