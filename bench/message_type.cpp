#include "bench.hpp"


namespace {


using namespace turner_bench;


// message_try_as_valid {{{1


template <typename Protocol>
void message_try_as_valid (benchmark::State &state)
{
  auto data = wire_data<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }

  auto request = msg->try_as(message_type<Protocol>);
  if (!request)
  {
    state.SkipWithError("unexpected message type");
  }

  for (auto _: state)
  {
    request = msg->try_as(message_type<Protocol>);
    benchmark::DoNotOptimize(request);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(message_try_as_valid, MSTURN);
BENCHMARK_TEMPLATE(message_try_as_valid, STUN);
BENCHMARK_TEMPLATE(message_try_as_valid, TURN);


// message_try_as_invalid {{{1


template <typename Protocol>
void message_try_as_invalid (benchmark::State &state)
{
  auto data = wire_data<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }

  auto request = msg->try_as(another_message_type<Protocol>);
  if (request)
  {
    state.SkipWithError("unexpected message type");
  }

  for (auto _: state)
  {
    request = msg->try_as(another_message_type<Protocol>);
    benchmark::DoNotOptimize(request);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(message_try_as_invalid, MSTURN);
BENCHMARK_TEMPLATE(message_try_as_invalid, STUN);
BENCHMARK_TEMPLATE(message_try_as_invalid, TURN);


// message_as_valid {{{1


template <typename Protocol>
void message_as_valid (benchmark::State &state)
{
  auto data = wire_data<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }
  (void)msg->as(message_type<Protocol>);

  for (auto _: state)
  {
    auto &request = msg->as(message_type<Protocol>);
    benchmark::DoNotOptimize(request);
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(message_as_valid, MSTURN);
BENCHMARK_TEMPLATE(message_as_valid, STUN);
BENCHMARK_TEMPLATE(message_as_valid, TURN);


// message_as_invalid {{{1


template <typename Protocol>
void message_as_invalid (benchmark::State &state)
{
  auto data = wire_data<Protocol>;

  std::error_code error;
  auto msg = Protocol::parse(data, error);
  if (error)
  {
    state.SkipWithError("unexpected error");
  }
  try
  {
    (void)msg->as(another_message_type<Protocol>);
    state.SkipWithError("unexpected error");
  }
  catch (...)
  {
    // expected
  }

  for (auto _: state)
  {
    try
    {
      auto &request = msg->as(another_message_type<Protocol>);
      benchmark::DoNotOptimize(request);
    }
    catch (...)
    {
      // expected
    }
  }

  state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(message_as_invalid, MSTURN);
BENCHMARK_TEMPLATE(message_as_invalid, STUN);
BENCHMARK_TEMPLATE(message_as_invalid, TURN);


// }}}1


} // namespace
