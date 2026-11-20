get_filename_component(_turner_fuzz_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

macro(turner_fuzz_setup)
    set(project_root "${_turner_fuzz_dir}")
    include("${project_root}/turner/list.cmake")

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(preset macos-clang-fuzz)
    else()
        set(preset linux-clang-fuzz)
    endif()

    set(build_dir "${project_root}/.build/${preset}")

    if(NOT IS_DIRECTORY "${build_dir}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} --preset ${preset}
            WORKING_DIRECTORY "${project_root}"
            RESULT_VARIABLE result
        )
        if(result)
            message(FATAL_ERROR "Configure failed for ${preset}")
        endif()
    endif()

    set(fuzz_exes)
    set(fuzz_corpus_dirs)
    set(fuzz_seed_dirs)
    set(fuzz_dictionaries)
    foreach(source IN LISTS turner_fuzz_sources)
        if(NOT source MATCHES "\\.fuzz\\.cpp$")
            continue()
        endif()
        string(REGEX REPLACE "\\.fuzz\\.cpp$" "" fuzz_path "${source}")
        string(REPLACE "/" "-" fuzz_exe "fuzz-${fuzz_path}")
        list(APPEND fuzz_exes "${fuzz_exe}")
        list(APPEND fuzz_corpus_dirs "${project_root}/fuzz/${fuzz_path}/corpus")
        list(APPEND fuzz_seed_dirs "${project_root}/fuzz/${fuzz_path}/seed")
        list(APPEND fuzz_dictionaries "${project_root}/fuzz/${fuzz_path}/dictionary")
    endforeach()

    execute_process(
        COMMAND ninja --quiet ${fuzz_exes}
        WORKING_DIRECTORY "${build_dir}"
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Build failed")
    endif()
endmacro()

function(turner_fuzz_dictionary_args out dictionary)
    if(EXISTS "${dictionary}")
        set(${out} "-dict=${dictionary}" PARENT_SCOPE)
    else()
        set(${out} "" PARENT_SCOPE)
    endif()
endfunction()

function(turner_fuzz_single source)
    set(project_root "${_turner_fuzz_dir}")

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(preset macos-clang-fuzz)
    else()
        set(preset linux-clang-fuzz)
    endif()

    set(build_dir "${project_root}/.build/${preset}")

    if(NOT IS_DIRECTORY "${build_dir}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} --preset ${preset}
            WORKING_DIRECTORY "${project_root}"
            RESULT_VARIABLE result
        )
        if(result)
            message(FATAL_ERROR "Configure failed for ${preset}")
        endif()
    endif()

    string(REGEX REPLACE "\\.fuzz\\.cpp$" "" fuzz_path "${source}")
    string(REPLACE "/" "-" fuzz_exe "fuzz-${fuzz_path}")
    set(corpus_dir "${project_root}/fuzz/${fuzz_path}/corpus")
    set(seed_dir "${project_root}/fuzz/${fuzz_path}/seed")
    turner_fuzz_dictionary_args(dictionary_args "${project_root}/fuzz/${fuzz_path}/dictionary")

    execute_process(
        COMMAND ninja --quiet ${fuzz_exe}
        WORKING_DIRECTORY "${build_dir}"
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Build failed for ${fuzz_exe}")
    endif()

    file(MAKE_DIRECTORY "${corpus_dir}")

    execute_process(
        COMMAND "${build_dir}/${fuzz_exe}" "${corpus_dir}" "${seed_dir}" -max_total_time=60 ${dictionary_args}
        WORKING_DIRECTORY "${build_dir}"
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Fuzz run failed for ${fuzz_exe} (crash?)")
    endif()
endfunction()

if(DEFINED TURNER_FUZZ_SOURCE)
    turner_fuzz_single("${TURNER_FUZZ_SOURCE}")
endif()
