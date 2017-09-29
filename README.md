# STUN/TURN client/server

Turner is multiplatform (Linux, MacOS, Windows) STUN/TURN protocol based
relay.


## Compiling and installing

    $ mkdir build && cd build
    $ cmake .. [-DTURNER_UNITTESTS=yes|no]


## Source tree

The source tree is organised as follows:

    .                   Root directory
    |-- cmake           CMake helpers
    |-- scripts         Helper scripts
    |-- turner          Library
    |   |-- protocol      STUN/TURN parser/generator
    |   |-- client        embeddable client
    |   `-- server        embeddable server
    |-- turner_client   client executable
    `-- turner_server   server executable
