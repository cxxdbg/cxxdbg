# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# CLI test
add_executable(dbgclitest
               app_command_interpreter_test.cpp
               exec_command_test.cpp
               break_base_group_test.cpp
               break_group_test.cpp
               command_group_test.cpp
               command_interpreter_test.cpp
               frame_group_test.cpp
               function_command_test.cpp
               mock_break_processor.hpp
               mock_code_break_processor.hpp
               mock_exec_processor.hpp
               mock_frame_processor.hpp
               mock_platform_processor.hpp
               mock_process_processor.hpp
               mock_target_processor.hpp
               mock_thread_processor.hpp
               mock_watch_processor.hpp
               platform_group_test.cpp
               process_group_test.cpp
               po_command_test.cpp
               target_group_test.cpp
               test.cpp
               thread_group_test.cpp
               watch_group_test.cpp
              )

target_link_libraries(dbgclitest PRIVATE cxxdbg-dbg-cli cxxdbg-async Boost::unit_test_framework)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(dbgclitest PRIVATE rt)
endif()

add_test(NAME dbgclitest COMMAND dbgclitest)
