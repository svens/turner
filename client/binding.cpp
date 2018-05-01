#include <client/main.hpp>
#include <turner/stun/client.hpp>
#include <turner/transport/tcp.hpp>
#include <turner/transport/udp.hpp>
#include <sal/net/ip/address.hpp>
#include <sal/program_options/command_line.hpp>
#include <sal/program_options/option_set.hpp>
#include <iostream>


namespace {


using namespace std::chrono_literals;


sal::net::ip::address_t server = sal::net::ip::make_address("127.0.0.1");
uint16_t server_port = 3478;
std::string transport = "udp";
std::chrono::seconds wait_timeout = 3s;


bool parse (int argc, const char *argv[])
{
  using namespace sal::program_options;

  option_set_t options;
  options
    .add({"h", "help"},
      help("Print this helpful screen")
    )
    .add({"s", "server"},
      requires_argument("IP", server),
      help("Server IP address\nDefault is 127.0.0.1")
    )
    .add({"p", "port"},
      requires_argument("INT", server_port),
      help("Server port\nDefault is " + std::to_string(server_port))
    )
    .add({"t", "transport"},
      requires_argument("STRING", transport),
      help("Transport protocol (udp, tcp)\nDefault is " + transport)
    )
    .add({"w", "wait"},
      requires_argument("INT", wait_timeout.count()),
      help("Max response waiting time in seconds\n"
        "Default is " + std::to_string(wait_timeout.count())
      )
    )
  ;
  auto arguments = options.parse<command_line_t>(argc, argv);

  // --help
  if (arguments.has("help"))
  {
    std::cout
      << "usage:\n"
      << "  " << argv0 << " binding [arguments]\n\n"
      << "arguments:"
      << options
      << '\n';
    return false;
  }

  // --server
  server = sal::net::ip::make_address(
    options.back_or_default("server", {arguments})
  );

  // --port
  server_port = static_cast<uint16_t>(
    std::stoul(options.back_or_default("port", {arguments}))
  );

  // --transport
  transport = options.back_or_default("transport", {arguments});

  // --wait
  wait_timeout = std::chrono::seconds(
    std::stoul(options.back_or_default("wait", {arguments}))
  );

  return true;
}


} // namespace


template <typename Transport>
int binding_impl ()
{
  turner::stun::client_t<Transport> client(server, server_port);
  client.timeout(wait_timeout);

  auto [address, port] = client.binding();
  if (address.is_v4())
  {
    std::cout << address << ':' << port << '\n';
  }
  else
  {
    std::cout << '[' << address << "]:" << port << '\n';
  }

  return EXIT_SUCCESS;
}


int binding (int argc, const char *argv[])
{
  if (!parse(argc, argv))
  {
    return EXIT_SUCCESS;
  }

  if (transport == "udp")
  {
    return binding_impl<turner::transport::udp_t>();
  }
  else if (transport == "tcp")
  {
    return binding_impl<turner::transport::tcp_t>();
  }

  sal::throw_runtime_error("invalid transport: '" + transport + '\'');
}
