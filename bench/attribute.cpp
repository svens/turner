#include "bench.hpp"


namespace {


using namespace turner_bench;


// attribute_read_valid {{{1


template <typename Protocol>
void attribute_read_valid (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    auto attribute = request.read(attribute_type<Protocol>, error);
    if (error)
    {
      throw std::logic_error("unexpectd error: " + error.message());
    }

    for (auto _: state)
    {
      attribute = request.read(attribute_type<Protocol>, error);
      benchmark::DoNotOptimize(attribute);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(attribute_read_valid, MSTURN);
BENCHMARK_TEMPLATE(attribute_read_valid, STUN);
BENCHMARK_TEMPLATE(attribute_read_valid, TURN);


// attribute_read_invalid {{{1


template <typename Protocol>
void attribute_read_invalid (benchmark::State &state)
{
  try
  {
    auto data = wire_data_with_payload(Protocol(),
      "\x00\x0d\x00\x03"
      "\x00\x00\x02\x58"
    );

    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    auto attribute = request.read(attribute_type<Protocol>, error);
    if (error != turner::errc::unexpected_attribute_length)
    {
      throw std::logic_error("unexpected error: " + error.message());
    }

    for (auto _: state)
    {
      attribute = request.read(attribute_type<Protocol>, error);
      benchmark::DoNotOptimize(attribute);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(attribute_read_invalid, MSTURN);
BENCHMARK_TEMPLATE(attribute_read_invalid, STUN);
BENCHMARK_TEMPLATE(attribute_read_invalid, TURN);


// attribute_read_missing {{{1


template <typename Protocol>
void attribute_read_missing (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    auto attribute = request.read(another_attribute_type<Protocol>, error);
    if (error != turner::errc::attribute_not_found)
    {
      throw std::logic_error("unexpected error: " + error.message());
    }

    for (auto _: state)
    {
      attribute = request.read(another_attribute_type<Protocol>, error);
      benchmark::DoNotOptimize(attribute);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(attribute_read_missing, MSTURN);
BENCHMARK_TEMPLATE(attribute_read_missing, STUN);
BENCHMARK_TEMPLATE(attribute_read_missing, TURN);


// attribute_write {{{1


template <typename Protocol>
void attribute_write (benchmark::State &state)
{
  try
  {
    auto data = wire_data<Protocol>;
    auto msg = Protocol::parse(data);
    auto &request = msg->as(message_type<Protocol>);

    std::error_code error;
    request
      .to_success_response(data, error)
      .write(attribute_type<Protocol>, 600s)
    ;
    if (error)
    {
      throw std::logic_error("unexpectd error: " + error.message());
    }

    for (auto _: state)
    {
      auto writer = request
        .to_success_response(data, error)
        .write(attribute_type<Protocol>, 600s)
      ;
      benchmark::DoNotOptimize(writer);
    }

    state.SetItemsProcessed(state.iterations());
  }
  catch (const std::exception &e)
  {
    state.SkipWithError(e.what());
  }
}

BENCHMARK_TEMPLATE(attribute_write, MSTURN);
BENCHMARK_TEMPLATE(attribute_write, STUN);
BENCHMARK_TEMPLATE(attribute_write, TURN);


// }}}1


} // namespace
