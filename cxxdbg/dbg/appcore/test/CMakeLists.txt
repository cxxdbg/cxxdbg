# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# appcore library test
add_executable(appcoretest
               fmt_result_tree.hpp
               test.cpp
               watch_list_server_test.cpp)

target_link_libraries(appcoretest PRIVATE cxxdbg-dbg-appcore Boost::unit_test_framework)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(appcoretest PRIVATE rt)
endif()

add_test(NAME appcoretest COMMAND appcoretest)
