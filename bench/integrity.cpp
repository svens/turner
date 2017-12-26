#include "bench.hpp"


namespace {


using namespace turner_bench;


// integrity_valid {{{1


template <typename Protocol>
void integrity_valid (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto hmac = integrity_calculator(Protocol());

    bool ok = msg->has_valid_integrity(hmac);
    if (!ok)
    {
      throw std::logic_error("unexpected invalid message integrity");
    }

    std::error_code error;
    for (auto _: state)
    {
      ok = msg->has_valid_integrity(hmac, error);
      benchmark::DoNotOptimize(ok);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(integrity_valid, MSTURN);
BENCHMARK_TEMPLATE(integrity_valid, STUN);
BENCHMARK_TEMPLATE(integrity_valid, TURN);


// integrity_invalid {{{1


template <typename Protocol>
void integrity_invalid (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    data.back() ^= 1;

    auto msg = Protocol::parse(data);
    auto hmac = integrity_calculator(Protocol());

    std::error_code error;
    bool ok = msg->has_valid_integrity(hmac, error);
    if (error != turner::errc::unexpected_attribute_value)
    {
      sal::throw_system_error(error, "expected invalid integrity");
    }

    for (auto _: state)
    {
      ok = msg->has_valid_integrity(hmac, error);
      benchmark::DoNotOptimize(ok);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(integrity_invalid, MSTURN);
BENCHMARK_TEMPLATE(integrity_invalid, STUN);
BENCHMARK_TEMPLATE(integrity_invalid, TURN);


// integrity_missing {{{1


template <typename Protocol>
void integrity_missing (benchmark::State &state)
{
  try
  {
    auto data = wire_data_with_payload(Protocol(), "");
    auto msg = Protocol::parse(data);
    auto hmac = integrity_calculator(Protocol());

    std::error_code error;
    bool ok = msg->has_valid_integrity(hmac, error);
    if (error != turner::errc::attribute_not_found)
    {
      sal::throw_system_error(error, "expected missing integrity");
    }

    for (auto _: state)
    {
      ok = msg->has_valid_integrity(hmac, error);
      benchmark::DoNotOptimize(ok);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(integrity_missing, MSTURN);
BENCHMARK_TEMPLATE(integrity_missing, STUN);
BENCHMARK_TEMPLATE(integrity_missing, TURN);


// }}}1


} // namespace
