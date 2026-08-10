# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Process library test
add_executable(proctest
               bp_child_monitor_test.cpp
               bp_io_launcher_test.cpp
               test.cpp
               mock_io_launcher.hpp
              )

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_sources(proctest PRIVATE win32_monitor_test.cpp)
else()
    target_sources(proctest PRIVATE posix_monitor_test.cpp
                                    bp_pty_launcher_test.cpp)
endif()

target_link_libraries(proctest PUBLIC cxxdbg-proc cxxdbg-async cxxdbg-util)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(proctest PRIVATE rt)
endif()


add_test(NAME proctest COMMAND proctest)
