# STUN/TURN/MSTURN

[STUN]: https://tools.ietf.org/html/rfc5389
[TURN]: https://tools.ietf.org/html/rfc5766
[MSTURN]: https://msdn.microsoft.com/en-us/library/cc431507(v=office.12).aspx

[![Build Status](https://travis-ci.org/svens/turner.svg?branch=master)](https://travis-ci.org/svens/turner)
[![Build Status](https://ci.appveyor.com/api/projects/status/76xm7ukgm9enegjx/branch/master?svg=true)](https://ci.appveyor.com/project/svens/turner/branch/master)
[![Coverage](https://coveralls.io/repos/github/svens/turner/badge.svg?branch=master)](https://coveralls.io/github/svens/turner?branch=master)

[STUN]/[TURN]/[MSTURN] parser/generator library


## Documentation

  * [STUN]
  * [TURN]
  * [MSTURN]


## Compiling and installing

    $ mkdir build && cd build
    $ cmake .. [-DUNITTESTS=yes|no] [-DDOCS=yes|no]


## Source tree

The source tree is organised as follows:

    .                   root directory
    |-- cmake           CMake helpers
    |-- scripts         helper scripts
    `-- turner          protocol parser library
