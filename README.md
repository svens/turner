# turner

C++23 STUN/TURN/MS-TURN library and server.

[![CI](https://github.com/svens/turner/actions/workflows/ci.yml/badge.svg)](https://github.com/svens/turner/actions/workflows/ci.yml)
[![codecov](https://codecov.io/github/svens/turner/graph/badge.svg)](https://codecov.io/github/svens/turner)

## Directory structure

    .                       Root of source tree
    |- cmake/               Build system support modules
    |  `- actions/          Cross-platform action scripts (invoked via ninja)
    |- app/                 Hosts ...
    |  `- <component>       ... running a turner/<component> on pal I/O
    |- turner/              Library ...
    |  `- <component>       ... per component headers, sources and tests
    |- CMakeLists.txt       Top-level CMake project
    |- CMakePresets.json    Compiler/config presets and workflows
    `- build.ninja          Convenience wrapper for cmake workflow presets

## Dependencies

1. [pal](https://github.com/svens/pal): fetched at the commit pinned in
   `cmake/pal.cmake`. A sibling checkout at `../../pal/main` is used instead
   when present; set `FETCHCONTENT_SOURCE_DIR_PAL` to choose another tree.

## Building

    ninja help              # show available targets
    ninja                   # build and test all compiler/config combinations (platform-specific)

Each target delegates to `cmake -P cmake/actions/<target>.cmake`.
