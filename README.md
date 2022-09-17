# STUN/TURN/MSTURN

[STUN]: https://tools.ietf.org/html/rfc8489
[TURN]: https://tools.ietf.org/html/rfc8656
[MS-TURN]: https://docs.microsoft.com/en-us/openspecs/office_protocols/ms-turn
[Turner]: http://turner.alt.ee

[![Build](https://github.com/svens/turner/workflows/Build/badge.svg)](https://github.com/svens/turner/actions?query=workflow:Build)
[![Coverage](https://coveralls.io/repos/github/svens/turner/badge.svg)](https://coveralls.io/github/svens/turner)

[STUN]/[TURN]/[MS-TURN] library

## Goals

  * STUN/TURN/MS-TURN protocol reader/writer (in progress)
    + Type-safe:
      - different protocol messages should be distinct types, cross-protocol
        mixups prevented at compile-time
      - attributes should be distinct types e.g. compiler should prevent
        application developer treating LIFETIME attribute as string but
        enforce std::chrono::seconds, etc.
    + No allocations:
      - extracting protocol level information from incoming networking
        datagram should not do any allocations
    + Zero-copy:
      - querying memory blob kind of attributes from messages should return
        std::span into original networking datagram
  * Client & server as library (TODO)
    + Implement OS independent pure business logic layer that provides
      hooks/callbacks for applications that implement actual syscalls
      (networking I/O, logging, monitoring, etc.)
    + Provide hooks to introduce artificial delays within business logic to
      support testing/debugging threading issues
  * Cross-platform relay (TODO)
    + Create actual application providing syscalls for relay implemented in library scope


## Documentation

  * [STUN]
  * [TURN]
    - [TURN extension for IPv6](https://tools.ietf.org/html/rfc6156)
    - [TURN extension for TCP Allocations](https://tools.ietf.org/html/rfc6062)
  * [MS-TURN], v18.0
  * [Turner]


## Compiling and installing

    $ mkdir build && cd build
    $ cmake .. [-Dturner_test=yes|no] [-Dturner_bench=yes|no] [-Dturner_doc=yes|no]
    $ make && make test && make install


## Source tree

The source tree is organised as follows:

    .               Root of source tree
    |- turner       Library ...
    |  `- module    ... per module headers/sources/tests
    |- bench        Benchmarks
    |- cmake        CMake modules
    |- pal          OS abstraction helper library
    `- samples      Sample applications using Turner library
