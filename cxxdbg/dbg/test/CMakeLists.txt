# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Application test
add_executable(dbgtest
               breakpoint_test.cpp
               call_stack_model_test.cpp
               code_breakpoint_test.cpp
               composite_source_tree_model_test.cpp
               custom_watch_list_test.cpp
               custom_watch_model_test.cpp
               debugger_terminal_test.cpp
               launch_options_test.cpp
               mock_custom_watch_list_impl.hpp
               recent_file_list_test.cpp
               source_tree_test.cpp
               test.cpp
               test_watch_list_impl.cpp
               test_watch_list_impl.hpp
               thread_list_test.cpp
               thread_test.cpp
               watch_list_test.cpp
               watch_model_test.cpp
               source_tree_builder_test.cpp
               breakpoints_model_test.cpp
               watchpoint_test.cpp)

target_link_libraries(dbgtest PRIVATE cxxdbg-dbg cxxdbg-app cxxdbg-dbg-fmt)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(dbgtest PRIVATE rt)
endif()

add_test(NAME dbgtest COMMAND dbgtest)
