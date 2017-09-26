#
# GNU g++
#

set(CMAKE_CXX_FLAGS "-std=c++1z -Wall -Werror -Wextra -Weffc++ -Werror -pedantic -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -ggdb -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
  # Ninja redirects build output and prints it only on error
  # Redirection strips colorization, so let's force it here
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color")
endif()
