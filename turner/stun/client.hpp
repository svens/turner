#pragma once

/**
 * \file turner/stun/client.hpp
 * Simple synchronous STUN client.
 */


#include <turner/config.hpp>
#include <turner/basic_agent.hpp>
#include <turner/stun/attributes.hpp>
#include <turner/stun/messages.hpp>
#include <turner/stun/protocol.hpp>


__turner_begin


namespace stun {


/**
 * Simple synchronous STUN client. Only supported operation is STUN Binding.
 */
template <typename Transport>
class client_t
  : public basic_agent_t<protocol_t, Transport>
{
  using base_t = basic_agent_t<protocol_t, Transport>;

public:

  /**
   * Create new STUN client, passing \a arg to underlying transport
   * constructor.
   */
  template <typename... Arg>
  client_t (Arg &&...arg)
    : base_t(std::forward<Arg>(arg)...)
  { }


  /**
   * Send STUN Binding request and wait for response. On success, return
   * underlying transport local address as seen by server (i.e. server
   * reflexive address). On \a error returns unspecified address.
   */
  xor_mapped_address_t::value_t binding (std::error_code &error) noexcept
  {
    uint8_t buf[1024];
    if (auto writer = stun::binding.make(buf, error))
    {
      auto [first, last] = writer.finish();
      if (base_t::transport_.send(first, last, error))
      {
        if (auto reader = base_t::receive(error))
        {
          if (auto message = stun::binding_success(reader))
          {
            return message->read(xor_mapped_address, error);
          }
          else
          {
            error = make_error_code(errc::unexpected_message_type);
          }
        }
      }
    }
    return {};
  }


  /**
   * \copydoc binding(std::error_code &)
   * \throws std::system_error on failure
   */
  xor_mapped_address_t::value_t binding ()
  {
    return binding(sal::throw_on_error("stun::client::binding"));
  }
};


} // namespace stun


__turner_end
