# STUN/TURN client/server

[![Build Status](https://travis-ci.org/svens/turner.svg?branch=master)](https://travis-ci.org/svens/turner)
[![Build Status](https://ci.appveyor.com/api/projects/status/76xm7ukgm9enegjx/branch/master?svg=true)](https://ci.appveyor.com/project/svens/turner/branch/master)
[![Coverage](https://coveralls.io/repos/github/svens/turner/badge.svg?branch=master)](https://coveralls.io/github/svens/turner?branch=master)

Turner is multiplatform (Linux, MacOS, Windows) STUN/TURN protocol based
relay.


## Compiling and installing

    $ mkdir build && cd build
    $ cmake .. [-DTURNER_UNITTESTS=yes|no] [-DTURNER_DOCS=yes|no]


## Source tree

The source tree is organised as follows:

    .                   Root directory
    |-- cmake           CMake helpers
    |-- scripts         Helper scripts
    |-- turner          Protocol parser library
    |   |-- client        embeddable client
    |   `-- server        embeddable server
    |-- turner_client   client executable
    `-- turner_server   server executable
