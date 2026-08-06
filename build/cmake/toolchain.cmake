# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Default toolchain selection for CXXDBG
# This file is included in every project (cxxdbg, cxxdbg-llvm, cxxdbg-lldb, cxxdbg-deps)
# before the project command to select correct default compiler and flags
# depending on platform.

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    # Using clang + MinGW compiler on Windows if not set
    if ("${CMAKE_C_COMPILER}" STREQUAL "")
        set(CMAKE_C_COMPILER "gcc")
        set(CMAKE_CXX_COMPILER "g++")
    endif()
elseif("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Darwin")
    # Using system ar and ranlib utilities by defult.
    # We need this in case of custom llvm toolchain because llvm utilities don't
    # support universal binaries

    if(NOT DEFINED CMAKE_AR)
        set(CMAKE_AR "/usr/bin/ar" CACHE STRING "Path to ar utility")
    endif()

    if(NOT DEFINED CMAKE_RANLIB)
        set(CMAKE_RANLIB "/usr/bin/ranlib" CACHE STRING "Path to ranlib utility")
    endif()
endif()
