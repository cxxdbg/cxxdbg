# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Command line interface library
add_library(cxxdbg-dbg-cli ${LIBRARY_TYPE}
            exec_processor.cpp
            exec_processor.hpp
            app_command_interpreter.cpp
            app_command_interpreter.hpp
            app_processor.cpp
            app_processor.hpp
            break_base_group.cpp
            break_base_group.hpp
            break_properties.cpp
            break_properties.hpp
            break_group.cpp
            break_group.hpp
            break_processor.hpp
            code_break_processor.hpp
            exec_command.cpp
            exec_command.hpp
            command.cpp
            command.hpp
            command_interpreter.cpp
            command_interpreter.hpp
            forward.hpp
            frame_group.cpp
            frame_group.hpp
            frame_processor.hpp
            function_command.cpp
            function_command.hpp
            command_group.cpp
            command_group.hpp
            platform_group.cpp
            platform_group.hpp
            platform_processor.hpp
            process_group.cpp
            process_group.hpp
            process_processor.cpp
            process_processor.hpp
            po_command.cpp
            po_command.hpp
            target_group.cpp
            target_group.hpp
            target_processor.cpp
            target_processor.hpp
            thread_group.cpp
            thread_group.hpp
            thread_processor.hpp
            watch_processor.hpp
            watch_group.cpp
            watch_group.hpp
           )

install_cxxdbg_library(cxxdbg-dbg-cli)

target_link_libraries(cxxdbg-dbg-cli PUBLIC cxxdbg-async cxxdbg-util)
target_link_libraries(cxxdbg-dbg-cli PUBLIC Boost::program_options)

add_subdirectory(test)
