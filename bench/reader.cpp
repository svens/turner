#include "bench.hpp"


namespace {


using namespace turner_bench;


// reader_try_as_valid {{{1


template <typename Protocol>
void reader_try_as_valid (benchmark::State &state)
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

BENCHMARK_TEMPLATE(reader_try_as_valid, MSTURN);
BENCHMARK_TEMPLATE(reader_try_as_valid, STUN);
BENCHMARK_TEMPLATE(reader_try_as_valid, TURN);


// reader_try_as_invalid {{{1


template <typename Protocol>
void reader_try_as_invalid (benchmark::State &state)
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

BENCHMARK_TEMPLATE(reader_try_as_invalid, MSTURN);
BENCHMARK_TEMPLATE(reader_try_as_invalid, STUN);
BENCHMARK_TEMPLATE(reader_try_as_invalid, TURN);


// reader_response_new_region {{{1


template <typename Protocol>
void reader_response_new_region (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    char buf[1024];
    std::error_code error;
    request.to_success_response(buf, error);
    if (error)
    {
      throw std::logic_error("unexpectd error: " + error.message());
    }

    for (auto _: state)
    {
      auto writer = request.to_success_response(buf, error);
      benchmark::DoNotOptimize(writer);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(reader_response_new_region, MSTURN);
BENCHMARK_TEMPLATE(reader_response_new_region, STUN);
BENCHMARK_TEMPLATE(reader_response_new_region, TURN);


// reader_response_same_region {{{1


template <typename Protocol>
void reader_response_same_region (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    request.to_success_response(data, error);
    if (error)
    {
      throw std::logic_error("unexpectd error: " + error.message());
    }

    for (auto _: state)
    {
      auto writer = request.to_success_response(data, error);
      benchmark::DoNotOptimize(writer);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(reader_response_same_region, MSTURN);
BENCHMARK_TEMPLATE(reader_response_same_region, STUN);
BENCHMARK_TEMPLATE(reader_response_same_region, TURN);


// }}}1


} // namespace
