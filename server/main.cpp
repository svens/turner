#include <turner/protocol/stun.hpp>
#include <turner/server/server.hpp>


int main (int argc, const char *argv[])
{
  (void)argc;
  (void)argv;

  turner::server::run(turner::protocol::who());

  return EXIT_SUCCESS;
}
