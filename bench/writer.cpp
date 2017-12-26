#include "bench.hpp"


namespace {


using namespace turner_bench;


// writer_finish {{{1


template <typename Protocol>
void writer_finish (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    auto writer = request.to_success_response(data, error);
    if (error)
    {
      throw std::logic_error("unexpectd error: " + error.message());
    }

    for (auto _: state)
    {
      auto [begin, end] = writer.finish();
      benchmark::DoNotOptimize(begin);
      (void)end;
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(writer_finish, MSTURN);
BENCHMARK_TEMPLATE(writer_finish, STUN);
BENCHMARK_TEMPLATE(writer_finish, TURN);


// writer_finish_with_integrity {{{1


template <typename Protocol>
void writer_finish_with_integrity (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    auto hmac = integrity_calculator(Protocol());
    request.to_success_response(data).finish(hmac, error);
    if (error)
    {
      throw std::logic_error("unexpectd error: " + error.message());
    }

    for (auto _: state)
    {
      auto [begin, end] = request
        .to_success_response(data, error)
        .finish(hmac, error)
      ;
      benchmark::DoNotOptimize(begin);
      (void)end;
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(writer_finish_with_integrity, MSTURN);
BENCHMARK_TEMPLATE(writer_finish_with_integrity, STUN);
BENCHMARK_TEMPLATE(writer_finish_with_integrity, TURN);


// }}}1


} // namespace
