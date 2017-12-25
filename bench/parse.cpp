#include "bench.hpp"


namespace {


using namespace turner_bench;


// parse_valid {{{1


template <typename Protocol>
void parse_valid (benchmark::State &state)
{
  auto data = wire_data<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }

  for (auto _: state)
  {
    msg = Protocol::parse(data, error);
    benchmark::DoNotOptimize(msg);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_valid, MSTURN);
BENCHMARK_TEMPLATE(parse_valid, STUN);
BENCHMARK_TEMPLATE(parse_valid, TURN);


// parse_invalid {{{1


template <typename Protocol>
void parse_invalid (benchmark::State &state)
{
  auto data = wire_data<Protocol>;
  data[Protocol::traits_t::cookie_offset] ^= 1;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (!error)
  {
    state.SkipWithError("expected error");
    return;
  }

  for (auto _: state)
  {
    msg = Protocol::parse(data, error);
    benchmark::DoNotOptimize(msg);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_invalid, MSTURN);
BENCHMARK_TEMPLATE(parse_invalid, STUN);
BENCHMARK_TEMPLATE(parse_invalid, TURN);


// }}}1


} // namespace
