# STUN/TURN/MSTURN

[STUN]: https://tools.ietf.org/html/rfc5389
[TURN]: https://tools.ietf.org/html/rfc5766
[MS-TURN]: https://msdn.microsoft.com/en-us/library/cc431507(v=office.12).aspx
[Turner]: http://turner.alt.ee

[![Build](https://github.com/svens/turner/workflows/Build/badge.svg)](https://github.com/svens/turner/actions?query=workflow:Build)
[![Coverage](https://coveralls.io/repos/github/svens/turner/badge.svg)](https://coveralls.io/github/svens/turner)

[STUN]/[TURN]/[MS-TURN] parser/generator library


## Documentation

  * [STUN]
  * [TURN]
    - [TURN extension for IPv6](https://tools.ietf.org/html/rfc6156)
  * [MS-TURN]
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
