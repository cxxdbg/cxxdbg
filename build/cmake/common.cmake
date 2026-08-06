# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# CMake script containing common code for other projects such as processing
# the CXXDBG_DEPLOYMENT_TYPE option and setting defult values for common options


if("${CXXDBG_COMMON_INCLUDED}")
    return()
endif()

set(CXXDBG_COMMON_INCLUDED ON)


include(CheckCXXSourceRuns)


# checking value of the CXXDBG_DEPLOYMENT_TYPE option

if("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "")
    message(STATUS "CXXDBG_DEPLOYMENT_TYPE is not set, using Development by default")
    set(CXXDBG_DEPLOYMENT_TYPE "Development" CACHE STRING "Deployment type")
endif()

if("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "Release")
    message(STATUS "Selected Release deployment type")
elseif("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "Development")
    message(STATUS "Selected Development deployment type")
else()
    message(FATAL_ERROR "Unknown value of the CXXDBG_DEPLOYMENT_TYPE option: ${CXXDBG_DEPLOYMENT_TYPE}")
endif()


# setting default option values depending on deployment type
if("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "Development")
    set(CXXDBG_ENABLE_SHARED_DEFAULT ON)
    set(CXXDBG_ENABLE_SYSTEM_SHARED_DEFAULT ON)

    # shared libraries in LLVM should always be disabled for non-windows build
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(CXXDBG_LLVM_ENABLE_SHARED_DEFAULT ON)
    else()
        set(CXXDBG_LLVM_ENABLE_SHARED_DEFAULT OFF)
    endif()

    set(LLVM_ENABLE_SHARED_SYSLIBS_DEFAULT ON)
else()
    set(CXXDBG_ENABLE_SHARED_DEFAULT OFF)
    set(CXXDBG_ENABLE_SYSTEM_SHARED_DEFAULT OFF)
    set(CXXDBG_LLVM_ENABLE_SHARED_DEFAULT OFF)

    # Don't link system libraries (zlib) on Linux statically for now
    # because Qt depen on zlib
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        set(LLVM_ENABLE_SHARED_SYSLIBS_DEFAULT ON)
    else()
        set(LLVM_ENABLE_SHARED_SYSLIBS_DEFAULT OFF)
    endif()
endif()


# Checking if lld linker is available and works
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set(CXXDBG_USE_LLD_DEFAULT FALSE)
else()
    set(CMAKE_REQUIRED_FLAGS "-fuse-ld=lld")
    check_cxx_source_runs("int main() {}" CXXDBG_USE_LLD_DEFAULT)
    if("${CXXDBG_USE_LLD_DEFAULT}")
        message(STATUS "lld linker found, using -fuse-ld=lld option by default")
    else()
        message(STATUS "lld linker not found, not using -fuse-ld=lld option by default")
    endif()
    set(CMAKE_REQUIRED_FLAGS)
endif()


# Using libc++ standard library with Clang
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CXXDBG_USE_LIBCXX_DEFAULT ON)
else()
    set(CXXDBG_USE_LIBCXX_DEFAULT OFF)
endif()


# Using emulated tls on windows with Clang + mingw
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${MINGW}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -femulated-tls -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -femulated-tls -pthread")
endif()


# Don't use --gc-sections linker flags in llvm build with mingw because it's broken
if("${MINGW}")
    set(LLVM_NO_DEAD_STRIP ON CACHE BOOL "Don't strip unused sections")
endif()


# Libedit and ncurses does not work well on Windows for now
# TODO: Do we need these libraries on Windows
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(LLDB_ENABLE_LIBEDIT_DEFAULT FALSE)
    set(LLDB_ENABLE_CURSES_DEFAULT FALSE)
    set(LLVM_ENABLE_LIBEDIT_DEFAULT FALSE)
else()
    set(LLDB_ENABLE_LIBEDIT_DEFAULT FALSE)
    set(LLDB_ENABLE_CURSES_DEFAULT FALSE)
    set(LLVM_ENABLE_LIBEDIT_DEFAULT FALSE)
endif()


option(CXXDBG_ENABLE_SHARED "Use shared libraries instead of static" "${CXXDBG_ENABLE_SHARED_DEFAULT}")
option(CXXDBG_ENABLE_SYSTEM_SHARED "Use shared system libraries instead of static" "${CXXDBG_ENABLE_SYSTEM_SHARED_DEFAULT}")
option(CXXDBG_LLVM_ENABLE_SHARED "Enable shared libraries in LLVM" ${CXXDBG_LLVM_ENABLE_SHARED_DEFAULT})
option(LLVM_ENABLE_SHARED_SYSLIBS "Link system libraries dynamically" ${LLVM_ENABLE_SHARED_SYSLIBS_DEFAULT})
option(CXXDBG_USE_LLD "Use lld linker for linking" "${CXXDBG_USE_LLD_DEFAULT}")
option(CXXDBG_USE_LIBCXX "Use libc++ standard library" "${CXXDBG_USE_LIBCXX_DEFAULT}")

# Terminfo is a part of ncurses. The LLVM_ENABLE_TERMINFO option in LLDB
# is used only to determine if linking with libtinfo is required.
option(LLVM_ENABLE_TERMINFO "Term info enabled in LLVM" OFF)

option(LLDB_ENABLE_CURSES "Enable curses support in LLDB" "${LLDB_ENABLE_CURSES_DEFAULT}")
option(LLDB_ENABLE_LIBEDIT "Enable libedit support in LLDB" "${LLDB_ENABLE_LIBEDIT_DEFAULT}")
option(LLVM_ENABLE_LIBEDIT "Enable libedit support in LLVM" "${LLVM_ENABLE_LIBEDIT_DEFAULT}")
option(LIBCLANG_BUILD_STATIC "Enable static libclang" ON)
option(LLDB_ENABLE_LUA "Enable LUA" OFF)


if("${LINK_JOBS}" STREQUAL "")
    set(LINK_JOBS "1")
endif()
set_property(GLOBAL PROPERTY JOB_POOLS "linkpool=${LINK_JOBS}")
set(CMAKE_JOB_POOL_LINK linkpool)

# Enable LZMA for release deployment
if("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "Release")
    set(LLDB_ENABLE_LZMA ON CACHE BOOL "Enable LZMA support")
endif()


if("${CXXDBG_USE_LLD}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
elseif(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
endif()

# Setting special debugging options on Windows with mingw+gcc
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    if(MINGW AND "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        # TODO: should we always use -gsplit-dwarf with LLD on all platforms?
#        # Use -gslplit-dwarf option with mingw because executable files are to
#        # big to link on Windows without this option
#        if("${CXXDBG_USE_LLD}")
#            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gsplit-dwarf")
#            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gsplit-dwarf")
#        endif()

        # Using dwarf-4 debug info explicitly on Windows
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-4")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4")
    endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if("${CXXDBG_USE_LIBCXX}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else()
        if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR MINGW)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
        endif()
    endif()
else()
    if("${CXXDBG_USE_LIBCXX}")
        message(FATAL_ERROR "CXXDBG_USE_LIBCXX can be used only with Clang compiler")
    endif()
endif()


# Flag for linking libstdc++fs in modules where std::filesystem is used
set(CXXDBG_NEED_LINK_STDCXXFS OFF)
if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
   ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT "${CXXDBG_USE_LIBCXX}"))
   set(CXXDBG_NEED_LINK_STDCXXFS ON)
endif()


if(NOT "${CXXDBG_ENABLE_SYSTEM_SHARED}")
    # linking with static libgcc and static libstdc++
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
    endif()

    # don't use -static-libstdc++ flag with clang and libc++ on Linux
    if (NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" AND "${CXXDBG_USE_LIBCXX}"))
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        # Use static flag on Windows to link all system compiler libraries statically
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        
        # Setting this variable causes cmake to not add -Wl,-Bstatc -Wl,-Bdynamic for
        # static libraries. We need it even if -static is added in link flags
        set(CMAKE_LINK_SEARCH_START_STATIC TRUE)
    endif()
endif()

