include(FetchContent)

get_filename_component(_pal_sibling_dir "${PROJECT_SOURCE_DIR}/../../pal/main" ABSOLUTE)
if(NOT DEFINED FETCHCONTENT_SOURCE_DIR_PAL AND IS_DIRECTORY "${_pal_sibling_dir}")
	set(FETCHCONTENT_SOURCE_DIR_PAL "${_pal_sibling_dir}" CACHE PATH "Local pal source tree")
endif()

FetchContent_Declare(pal
	GIT_REPOSITORY https://github.com/svens/pal.git
	GIT_TAG 1aaa35b113de187499650d2c1ca98895da43c40c
)
FetchContent_MakeAvailable(pal)

if(FETCHCONTENT_SOURCE_DIR_PAL)
	message(STATUS "pal: ${FETCHCONTENT_SOURCE_DIR_PAL}")
else()
	message(STATUS "pal: fetched")
endif()
