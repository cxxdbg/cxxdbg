# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Utility test
add_executable(utiltest
               command_line_parser_test.cpp
               print_test.cpp
               template_test.cpp
               test.cpp
               version_test.cpp
              )

target_link_libraries(utiltest PRIVATE cxxdbg-util Boost::unit_test_framework)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(utiltest PRIVATE rt)
endif()

add_test(NAME utiltest COMMAND utiltest)
