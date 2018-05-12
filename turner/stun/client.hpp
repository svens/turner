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
 * This client is stateless, i.e. it does not cache idempotent responses.
 */
template <typename Transport>
class basic_client_t
  : public basic_agent_t<protocol_t, Transport>
{
  using base_t = basic_agent_t<protocol_t, Transport>;

public:

  /**
   * Create new STUN client, passing \a arg to underlying transport
   * constructor.
   */
  template <typename... Arg>
  basic_client_t (Arg &&...arg)
    : base_t(std::forward<Arg>(arg)...)
  { }


  /**
   * Send STUN Binding request and wait for response. On success, return
   * underlying transport local address as seen by server (i.e. server
   * reflexive address). On \a error returns unspecified address.
   */
  xor_mapped_address_t::value_t binding (std::error_code &error) noexcept
  {
    if (base_t::send(stun::binding, error))
    {
      xor_mapped_address_t::value_t result;
      if (base_t::receive(stun::binding_success, error,
          xor_mapped_address.value_ref(result)))
      {
        return result;
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


/**
 * Simple synchronous STUN client. It will cache server idempotent responses
 * to decrease number of syscalls and network traffic.
 */
template <typename Transport>
class client_t
  : public basic_client_t<Transport>
{
public:

  /**
   * \copydoc basic_client_t::basic_client_t(Arg)
   */
  template <typename... Arg>
  client_t (Arg &&...arg)
    : basic_client_t<Transport>(std::forward<Arg>(arg)...)
  {}


  /**
   * Return underlying transport local address as seen by server. On \a error
   * returns unspecified address.
   */
  const xor_mapped_address_t::value_t &server_reflexive_address (
    std::error_code &error) noexcept
  {
    if (server_reflexive_.port)
    {
      error.clear();
    }
    else
    {
      server_reflexive_ = basic_client_t<Transport>::binding(error);
    }
    return server_reflexive_;
  }


  /**
   * \copydoc server_reflexive_address(std::error_code &)
   * \throws std::system_error on failure
   */
  const xor_mapped_address_t::value_t &server_reflexive_address ()
  {
    return server_reflexive_address(
      sal::throw_on_error("stun::client::server_reflexive_address")
    );
  }


private:

  xor_mapped_address_t::value_t server_reflexive_{};
};


} // namespace stun


__turner_end
