# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0048 NEW)

# Boost provides its own package config (BoostConfig.cmake); CMake's bundled
# FindBoost module is deprecated. Setting the CMAKE_POLICY_DEFAULT_CMP0167
# variable (rather than only cmake_policy(SET)) makes it also apply inside
# vendored third-party subdirectories that call their own
# cmake_minimum_required and would otherwise reset the policy to its warning
# default; the explicit cmake_policy(SET) below covers this directory's own
# scope, since it was already resolved by cmake_minimum_required() above
# before the default variable takes effect.
if(POLICY CMP0167)
    set(CMAKE_POLICY_DEFAULT_CMP0167 "NEW")
    cmake_policy(SET CMP0167 NEW)
endif()

# Selecting default toolchain and settings depending on platform
include("${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/toolchain.cmake")

project(cxxdbg)

include(CheckCXXSourceRuns)

include("${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/common.cmake")


# This global variables are used in modules from ./build/cmake/modules directory
# included from various parts of LLVM
set(CXXDBG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CXXDBG_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")


# setting default option values depending on deployment type
if("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "Development")
    set(CXXDBG_BUILD_DEPS_DEFAULT OFF)
    set(CXXDBG_BUILD_LLVM_DEFAULT OFF)
    set(CXXDBG_BUILD_LLDB_DEFAULT OFF)
    set(CXXDBG_LLVM_ENABLE_SHARED_DEFAULT ON)
    set(CXXDBG_DISABLE_WINDOWS_CONSOLE_DEFAULT OFF)
    set(CXXDBG_BUILD_LZMA_DEFAULT OFF)
else()
    set(CXXDBG_BUILD_DEPS_DEFAULT ON)
    set(CXXDBG_BUILD_LLVM_DEFAULT ON)
    set(CXXDBG_BUILD_LLDB_DEFAULT ON)
    set(CXXDBG_LLVM_ENABLE_SHARED_DEFAULT OFF)
    set(CXXDBG_DISABLE_WINDOWS_CONSOLE_DEFAULT ON)
    set(CXXDBG_BUILD_LZMA_DEFAULT OFF)
endif()

option(CXXDBG_BUILD_DEPS "Build dependencies (ncurses, libxml2, etc) from sources" "${CXXDBG_BUILD_DEPS_DEFAULT}")
option(CXXDBG_BUILD_LLVM "Build LLVM from sources instead of using external build" "${CXXDBG_BUILD_LLVM_DEFAULT}")
option(CXXDBG_BUILD_LLDB "Build LLDB from sources" "${CXXDBG_BUILD_LLDB_DEFAULT}")
option(CXXDBG_LLVM_ENABLE_SHARED "Enable shared libraries in LLVM and LLDB" "${CXXDBG_LLVM_ENABLE_SHARED_DEFAULT}")
option(CXXDBG_USE_RANGE_V3 "Use ranges-v3 library instead of std ranges" OFF)
option(CXXDBG_DISABLE_WINDOWS_CONSOLE "Disable console on Windows" "${CXXDBG_DISABLE_WINDOWS_CONSOLE_DEFAULT}")
option(CXXDBG_USE_QT6 "Use Qt6 for building" ON)
option(CXXDBG_ENABLE_CLI "Enable building of the cxxdbg-cli utility" OFF)

if("${CXXDBG_BUILD_LLVM}")
    set(CXXDBG_BUILD_LLDB ON)
endif()

if(NOT "${CXXDBG_BUILD_LLVM}" AND "${CXXDBG_BUILD_LLDB}")
    set(LLDB_ROOT "${CMAKE_CURRENT_BINARY_DIR}/cxxdbg-lldb")
endif()

add_compile_options(-Wno-unknown-warning-option)

# Building all dependencies
if("${CXXDBG_BUILD_DEPS}")
    add_subdirectory(build/cxxdbg-deps cxxdbg-deps)
endif()

# Adding paths to fake FindXXX modules for components built from sources

if("${CXXDBG_BUILD_LIBEDIT}")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/modules/LibEdit")
endif()

if("${CXXDBG_BUILD_NCURSES}")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/modules/Curses")
endif()

if("${CXXDBG_BUILD_BOOST}")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/modules/Boost")
endif()


# LLVM build
include("${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/llvm.cmake")


if(CXXDBG_ENABLE_SHARED)
    set(LIBRARY_TYPE "SHARED")
else()
    set(LIBRARY_TYPE "STATIC")
endif()


enable_testing()

include("${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/boost.cmake")


# workaround for linking shared boost libraries on windows with LLD.
# LLD requres export library as input argument, not dll itself (as GNU linker)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    foreach(lib ${boost_libraries})
        foreach(type "" "_DEBUG" "_RELEASE")
            get_property(loc TARGET Boost::${lib} PROPERTY IMPORTED_LOCATION${type})
            get_filename_component(ext "${loc}" EXT)
            if("${ext}" STREQUAL ".dll")
                set_property(TARGET Boost::${lib} PROPERTY IMPORTED_LOCATION${type} "${loc}.a")
            endif()
        endforeach()
    endforeach()
endif()

# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

if(MINGW)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols")
endif()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fstandalone-debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstandalone-debug")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -femit-class-debug-always")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -femit-class-debug-always")
endif()


set(CMAKE_CXX_STANDARD 20)


# function for adding install rules for libraries. Installs libraries
# only for shared library build. Defined before add_subdirectory(thirdparty)
# and add_subdirectory(libs) since both call it for their submodules.
function(install_cxxdbg_library targ)
    if(NOT CXXDBG_ENABLE_SHARED)
        return()
    endif()

    # install anything in CxxDbg.app subdirectory on Macos
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        set(inst_prefix "CxxDbg.app/Contents/Frameworks")
    else()
        set(inst_prefix "lib/cxxdbg/lib")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        set_target_properties("${targ}" PROPERTIES INSTALL_RPATH "@loader_path")
    else()
        set_target_properties("${targ}" PROPERTIES INSTALL_RPATH "$ORIGIN")
    endif()

    install(TARGETS "${targ}"
            RUNTIME DESTINATION "bin"
            LIBRARY DESTINATION "${inst_prefix}"
            COMPONENT cxxdbg)
endfunction()


# Third-party dependencies (json, range-v3, cm, tmvc)
add_subdirectory(thirdparty)

# In-house libraries developed alongside cxxdbg (cxxdap, cxxtn, dbgfmt).
# Added after thirdparty since dbgfmt depends on cm, added above.
add_subdirectory(libs)


# Ranges library
add_subdirectory(cxxdbg/ranges)


# CXXDBG build

set(CMAKE_MACOSX_RPATH TRUE)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    add_definitions(-Werror -Wno-deprecated-declarations -Wno-narrowing)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion")
endif()

# creating custom target for cxxdbg component install
add_custom_target(install-cxxdbg
                  COMMAND "${CMAKE_COMMAND}" "-D" "COMPONENT=cxxdbg" "-P" "cmake_install.cmake")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(cxxdbg)
add_subdirectory(resources/images)

# copy lldb-server or debuserver to bin and add install rules for it
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        set(LLDB_SERVER_NAME "debugserver")
        set(LLDB_SERVER_BUILD_NAME "debugserver")
        set(LLDB_SERVER_TARGET_NAME "debugserver")
    else()
        set(LLDB_SERVER_NAME "lldb-server")
        set(LLDB_SERVER_BUILD_NAME "lldb-server")
        set(LLDB_SERVER_TARGET_NAME "lldb-server")
    endif()

    set(LLDB_SERVER_EXE_NAME "${LLDB_SERVER_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    set(LLDB_SERVER_BUILD_EXE_NAME "${LLDB_SERVER_BUILD_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    if("${CXXDBG_BUILD_LLVM}")
        set(input "${CMAKE_CURRENT_BINARY_DIR}/cxxdbg-llvm/llvm/bin/${LLDB_SERVER_BUILD_EXE_NAME}")
    else()
        set(input "${LLDB_ROOT}/bin/${LLDB_SERVER_BUILD_EXE_NAME}")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        set(LLDB_SERVER_PATH "${CMAKE_CURRENT_BINARY_DIR}/CxxDbg.app/Contents/bin/${LLDB_SERVER_EXE_NAME}")
    else()
        set(LLDB_SERVER_PATH "${CMAKE_CURRENT_BINARY_DIR}/libexec/cxxdbg/${LLDB_SERVER_EXE_NAME}")
    endif()

    add_custom_command(OUTPUT "${LLDB_SERVER_PATH}"
                       COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${input}" "${LLDB_SERVER_PATH}"
                       DEPENDS "${input}")
    add_custom_target(lldb-server-stage ALL DEPENDS "${LLDB_SERVER_PATH}")

    if("${CXXDBG_BUILD_LLVM}" OR "${CXXDBG_BUILD_LLDB}")
        add_dependencies(lldb-server-stage "${LLDB_SERVER_TARGET_NAME}")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        set(inst_prefix "CxxDbg.app/Contents/bin")
    else()
        set(inst_prefix "libexec/cxxdbg")
    endif()

    if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        install(PROGRAMS "${LLDB_SERVER_PATH}" DESTINATION "${inst_prefix}" COMPONENT "cxxdbg")
    endif()
endif()


include("${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/cpack.cmake")
