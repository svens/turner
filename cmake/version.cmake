find_package(Git)
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --match "v[0-9]*.[0-9]*.[0-9]*" --abbrev=0
    RESULT_VARIABLE exit_code
    OUTPUT_VARIABLE PROJECT_VERSION
    ERROR_QUIET
  )
  if(${exit_code})
    set(PROJECT_VERSION "0.0.0")
  else()
    string(STRIP ${PROJECT_VERSION} PROJECT_VERSION)
    string(REGEX REPLACE "^v" "" PROJECT_VERSION ${PROJECT_VERSION})
  endif()
else()
  set(PROJECT_VERSION "0.0.0")
endif()
