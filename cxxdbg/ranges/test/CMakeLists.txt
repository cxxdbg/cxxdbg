# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Ranges test
add_executable(rangestest
               concat_test.cpp
               join_delim_test.cpp
               test.cpp)

target_link_libraries(rangestest PRIVATE ranges)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(rangestest PRIVATE rt)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(rangestest PRIVATE -fconcepts-diagnostics-depth=10)
endif()

add_test(NAME rangestest COMMAND rangestest)
