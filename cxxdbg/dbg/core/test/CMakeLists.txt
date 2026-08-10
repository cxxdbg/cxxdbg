# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# core debugger library test
add_executable(dbgcoretest
               memory_disassembler_test.cpp
               regex_test.cpp
               regexp_lists_test.cpp
               test.cpp
              )

target_link_libraries(dbgcoretest PRIVATE cxxdbg-dbg-core)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(dbgcoretest PRIVATE rt)
endif()

add_test(NAME dbgcoretest COMMAND dbgcoretest)
