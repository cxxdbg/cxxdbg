# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Header-only third-party dependencies vendored as git submodules.
# Each is exposed as a plain INTERFACE target for the rest of the tree to
# link against.

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build/cmake/modules/NlohmannJson")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)

add_library(range-v3 INTERFACE)
target_include_directories(range-v3 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/range-v3/include")

# cm is a vendored git submodule with its own CMake project (Boost-dependent,
# not header-only). Only the core data-model (cm) and its C++ pretty-printer
# (cm-cxx) are needed; the Clang converter, source-model layer, and
# standalone tools are not used by cxxdbg. cm/CMakeLists.txt's own
# cmake_minimum_required(VERSION 3.6) means its option() calls only honor
# cache variables (CMP0077 OLD behavior), so these must be forced into the
# cache rather than set as plain variables.
set(CM_ENABLE_CXX ON CACHE BOOL "" FORCE)
set(CM_CXX_ENABLE_CLANG OFF CACHE BOOL "" FORCE)
set(CM_ENABLE_SRC OFF CACHE BOOL "" FORCE)
set(CM_ENABLE_TOOLS OFF CACHE BOOL "" FORCE)

# cm's targets (cm, cm-cxx, cm-log) use plain add_library() with no explicit
# STATIC/SHARED, so they follow BUILD_SHARED_LIBS like any other CMake
# target. Match cxxdbg's own CXXDBG_ENABLE_SHARED/LIBRARY_TYPE convention so they
# build shared exactly when the rest of cxxdbg's libraries do.
set(BUILD_SHARED_LIBS "${CXXDBG_ENABLE_SHARED}")
add_subdirectory(cm)
unset(BUILD_SHARED_LIBS)

# tmvc is a vendored git submodule with its own standalone CMake project,
# header-only. Its Qt5/Qt6 selection option (TMVC_USE_QT5) is the inverse of
# CXXDBG_USE_QT6, so bridge the two before adding it. Examples and tests are
# disabled now that the migration off cxxdbg/text is done; BUILD_TESTING only
# affects this directory's own subtree, so it doesn't touch other submodules'
# tests.
if(NOT CXXDBG_USE_QT6)
    set(TMVC_USE_QT5 ON)
endif()
set(TMVC_BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
add_subdirectory(tmvc)
