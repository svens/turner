include(FetchContent)

get_filename_component(_pal_sibling_dir "${PROJECT_SOURCE_DIR}/../../pal/main" ABSOLUTE)
if(NOT DEFINED FETCHCONTENT_SOURCE_DIR_PAL AND IS_DIRECTORY "${_pal_sibling_dir}")
	set(FETCHCONTENT_SOURCE_DIR_PAL "${_pal_sibling_dir}" CACHE PATH "Local pal source tree")
endif()

FetchContent_Declare(pal
	GIT_REPOSITORY https://github.com/svens/pal.git
	GIT_TAG cc7f387d2b2ceda5a298d783cc2fa702dddba940
)
FetchContent_MakeAvailable(pal)

if(FETCHCONTENT_SOURCE_DIR_PAL)
	message(STATUS "pal: ${FETCHCONTENT_SOURCE_DIR_PAL}")
else()
	message(STATUS "pal: fetched")
endif()
