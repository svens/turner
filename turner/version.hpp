#pragma once

/**
 * \file turner/version.hpp
 * Library version
 */

#include <string_view>

namespace turner
{

/// Library version string (MAJOR.MINOR.PATCH[-HASH][-dirty])
extern const std::string_view version;

/// Library major version
extern const int version_major;

/// Library minor version
extern const int version_minor;

/// Library patch version
extern const int version_patch;

} // namespace turner
