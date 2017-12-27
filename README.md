# STUN/TURN/MSTURN

[STUN]: https://tools.ietf.org/html/rfc5389
[TURN]: https://tools.ietf.org/html/rfc5766
[MS-TURN]: https://msdn.microsoft.com/en-us/library/cc431507(v=office.12).aspx
[Turner]: http://turner.alt.ee

[![Build Status](https://travis-ci.org/svens/turner.svg?branch=master)](https://travis-ci.org/svens/turner)
[![Build Status](https://ci.appveyor.com/api/projects/status/76xm7ukgm9enegjx/branch/master?svg=true)](https://ci.appveyor.com/project/svens/turner/branch/master)
[![Coverage](https://coveralls.io/repos/github/svens/turner/badge.svg?branch=master)](https://coveralls.io/github/svens/turner?branch=master)

[STUN]/[TURN]/[MS-TURN] parser/generator library


## Documentation

  * [STUN]
  * [TURN]
  * [MS-TURN]
  * [Turner]


## Compiling and installing

    $ mkdir build && cd build
    $ cmake .. [-Dturner_unittests=yes|no] [-Dturner_bench=yes|no] [-Dturner_docs=yes|no]


## Source tree

The source tree is organised as follows:

    .                   root directory
    |-- bench           library micro-benchmarking application
    |-- cmake           CMake helpers
    |-- docs            additional documentation
    |-- scripts         helper scripts
    |-- tps             3rd party libraries as git submodules
    `-- turner          protocol parser library
