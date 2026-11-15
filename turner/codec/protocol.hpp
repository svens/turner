#pragma once

/**
 * \file turner/codec/protocol.hpp
 * Protocol and entity relationship
 */

#include <concepts>
#include <type_traits>

namespace turner
{

/// True if \a Entity (method, message, attribute) is usable with \a Protocol: it is defined by \a Protocol or its base.
template <typename Entity, typename Protocol>
concept usable_with = std::derived_from<Protocol, typename std::remove_cvref_t<Entity>::protocol_type>;

} // namespace turner
