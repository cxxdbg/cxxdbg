# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# boost.cmake
# Detects boost and required boost libraries


if("${CXXDBG_ENABLE_SHARED}")
    set(CXXDBG_ENABLE_SHARED_BOOST_DEFAULT ON)
else()
    set(CXXDBG_ENABLE_SHARED_BOOST_DEFAULT OFF)
endif()

option(CXXDBG_ENABLE_SHARED_BOOST "Use shared boost libraries" "${CXXDBG_ENABLE_SHARED_BOOST_DEFAULT}")


if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(Boost_ARCHITECTURE "-x64")
endif()

if("${CXXDBG_ENABLE_SHARED_BOOST}")
    set(Boost_USE_STATIC_LIBS OFF)
else()
    set(Boost_USE_STATIC_LIBS ON)
endif()

if(NOT "${CXXDBG_ENABLE_SHARED}" AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(Boost_USE_STATIC_RUNTIME ON)
else()
    set(Boost_USE_STATIC_RUNTIME OFF)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(Boost_NO_BOOST_CMAKE ON)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_definitions("-DBOOST_USE_WINDOWS_H=1" "-DWIN32_LEAN_AND_MEAN=1")
endif()

set(Boost_NO_WARN_NEW_VERSIONS ON)


set(boost_libraries
    date_time
    regex
    filesystem
    serialization
    program_options
    log
    log_setup
    thread
    chrono
    atomic
    iostreams
    unit_test_framework)

# Only mingw64's Boost package ships a boost_dll CMake config
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    list(APPEND boost_libraries dll)
endif()

find_package(Boost 1.74 REQUIRED COMPONENTS ${boost_libraries})

# Boost.Process, Boost.Interprocess, Boost.System and (outside Windows)
# Boost.DLL are header-only and have no compiled component, so the system
# BoostConfig.cmake doesn't provide Boost:: targets for them (unlike the
# FetchContent-built Boost, which creates one per module).
if(NOT TARGET Boost::process)
    add_library(Boost::process ALIAS Boost::headers)
endif()
if(NOT TARGET Boost::interprocess)
    add_library(Boost::interprocess ALIAS Boost::headers)
endif()
if(NOT TARGET Boost::system)
    add_library(Boost::system ALIAS Boost::headers)
endif()
if(NOT TARGET Boost::dll)
    add_library(Boost::dll ALIAS Boost::headers)
endif()

# We use the Boost.Process v1 API (unqualified boost::process::child/args/...).
# Boost >= 1.86 defaults to the new v2 API and only exposes v1 under
# boost::process::v1 unless BOOST_PROCESS_VERSION is defined
add_compile_definitions(BOOST_PROCESS_VERSION=1)
