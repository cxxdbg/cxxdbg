# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

add_library(ranges INTERFACE)

target_include_directories(ranges INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ranges INTERFACE range-v3)
target_compile_definitions(ranges INTERFACE "__cpp_coroutines=201703L")

if("${CXXDBG_USE_RANGE_V3}")
    # Using range v3 library
    target_compile_definitions(ranges INTERFACE "CXXDBG_USE_RANGE_V3=1")
endif()


add_subdirectory(test)
