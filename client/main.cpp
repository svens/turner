#include <turner/protocol/stun.hpp>
#include <turner/client/client.hpp>


int main (int argc, const char *argv[])
{
  (void)argc;
  (void)argv;

  turner::client::run(turner::protocol::who());

  return EXIT_SUCCESS;
}
