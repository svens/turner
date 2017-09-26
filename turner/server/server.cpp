#include <turner/server/server.hpp>
#include <iostream>


namespace turner { namespace server {


void run (std::string msg)
{
  std::cout << "turner_server: " << msg << '\n';
}


}} // namespace turner::server
