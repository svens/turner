#include "bench.hpp"


namespace {


using namespace turner_bench;


// parse_valid_with_error {{{1


template <typename Protocol>
void parse_valid_with_error (benchmark::State &state)
{
  auto data = raw_message<Protocol>;

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

BENCHMARK_TEMPLATE(parse_valid_with_error, MSTURN);
BENCHMARK_TEMPLATE(parse_valid_with_error, STUN);
BENCHMARK_TEMPLATE(parse_valid_with_error, TURN);


// parse_valid_with_exception {{{1


template <typename Protocol>
void parse_valid_with_exception (benchmark::State &state)
{
  auto data = raw_message<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }

  for (auto _: state)
  {
    msg = Protocol::parse(data);
    benchmark::DoNotOptimize(msg);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_valid_with_exception, MSTURN);
BENCHMARK_TEMPLATE(parse_valid_with_exception, STUN);
BENCHMARK_TEMPLATE(parse_valid_with_exception, TURN);


// parse_invalid_with_error {{{1


template <typename Protocol>
void parse_invalid_with_error (benchmark::State &state)
{
  auto data = raw_message<Protocol>;
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

BENCHMARK_TEMPLATE(parse_invalid_with_error, MSTURN);
BENCHMARK_TEMPLATE(parse_invalid_with_error, STUN);
BENCHMARK_TEMPLATE(parse_invalid_with_error, TURN);


// parse_invalid_with_exception {{{1


template <typename Protocol>
void parse_invalid_with_exception (benchmark::State &state)
{
  auto data = raw_message<Protocol>;
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
    try
    {
      msg = Protocol::parse(data);
      benchmark::DoNotOptimize(msg);
    }
    catch (...)
    {
      // ignore
    }
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(parse_invalid_with_exception, MSTURN);
BENCHMARK_TEMPLATE(parse_invalid_with_exception, STUN);
BENCHMARK_TEMPLATE(parse_invalid_with_exception, TURN);


// }}}1


} // namespace
