# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Async test
add_executable(asynctest
               event_queue_test.cpp
               result_test.cpp
               test.cpp
              )

target_link_libraries(asynctest PRIVATE cxxdbg-async)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(asynctest PRIVATE rt)
endif()

add_test(NAME asynctest COMMAND asynctest)
