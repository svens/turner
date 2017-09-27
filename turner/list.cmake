set(turner "")
set(turner_unittests "")

include(turner/protocol/list.cmake)
include(turner/client/list.cmake)
include(turner/server/list.cmake)

list(APPEND turner_unittests
  turner/common.test.hpp
)
