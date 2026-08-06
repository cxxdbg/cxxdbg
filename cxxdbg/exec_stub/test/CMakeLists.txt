# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# test for cxxdbg-dbg-execstub library

add_executable(execstubtest
               exec_stub_server_client_test.cpp
               exec_stub_struct_test.cpp
               test.cpp
              )


target_link_libraries(execstubtest PRIVATE cxxdbg-dbg-execstub Boost::unit_test_framework)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(execstubtest PRIVATE rt)
endif()

add_test(NAME execstubtest COMMAND execstubtest)
