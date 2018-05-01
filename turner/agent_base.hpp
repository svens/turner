#pragma once

/**
 * \file turner/agent_base.hpp
 * Common agent base class.
 */

#include <turner/config.hpp>


__turner_begin


/**
 * Common protocol independent agent information and functionality. Agent is
 * participant in TURN/MS-TURN session (i.e. either on client or server side).
 */
class agent_base_t
{
protected:

  ~agent_base_t () noexcept = default;
};


__turner_end
