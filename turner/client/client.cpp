#include <turner/client/client.hpp>
#include <iostream>


namespace turner { namespace client {


void run (std::string msg)
{
  std::cout << "turner_client: " << msg << '\n';
}


}} // namespace turner::client
