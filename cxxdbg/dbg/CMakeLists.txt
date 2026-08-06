# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

add_library(cxxdbg-dbg ${LIBRARY_TYPE}
            address_breakpoint.cpp
            address_breakpoint.hpp
            application.cpp
            application.hpp
            auto_completion_model.hpp
            auto_completion_model.cpp
            breakpoint.cpp
            breakpoint.hpp
            breakpoint_info.cpp
            breakpoint_info.hpp
            breakpoint_list.cpp
            breakpoint_list.hpp
            breakpoint_list_impl.hpp
            breakpoint_location.cpp
            breakpoint_location.hpp
            breakpoint_location_info.cpp
            breakpoint_location_info.hpp
            breakpoint_site.cpp
            breakpoint_site.hpp
            breakpoint_utils.hpp
            breakpoints_view_model.cpp
            breakpoints_view_model.hpp
            breakpoints_view_tree_node.cpp
            breakpoints_view_tree_node.hpp
            call_parameter.cpp
            call_parameter.hpp
            call_parameter_info.cpp
            call_parameter_info.hpp
            call_stack_model.cpp
            call_stack_model.hpp
            call_stack_tree.cpp
            call_stack_tree.hpp
            cli_processor.cpp
            cli_processor.hpp
            code_breakpoint.cpp
            code_breakpoint.hpp
            code_position.cpp
            code_position.hpp
            code_position_info.cpp
            code_position_info.hpp
            command_line.cpp
            command_line.hpp
            composite_source_tree_model.cpp
            composite_source_tree_model.hpp
            compile_unit_info.cpp
            compile_unit_info.hpp
            custom_watch_list.cpp
            custom_watch_list.hpp
            custom_watch_list_impl.hpp
            custom_watch_model.cpp
            custom_watch_model.hpp
            dbg_cli_processor.cpp
            dbg_cli_processor.hpp
            dbg_terminal.cpp
            dbg_terminal.hpp
            debugger.cpp
            debugger.hpp
            debugger_impl.hpp
            debugger_terminal.cpp
            debugger_terminal.hpp
            debug_settings.cpp
            debug_settings.hpp
            exception_caught_breakpoint.cpp
            exception_caught_breakpoint.hpp
            exception_thrown_breakpoint.cpp
            exception_thrown_breakpoint.hpp
            forward.hpp
            function_breakpoint.cpp
            function_breakpoint.hpp
            launch_options.cpp
            launch_options.hpp
            import.hpp
            memory_data_source.cpp
            memory_data_source.hpp
            memory_document.cpp
            memory_document.hpp
            module.cpp
            module.hpp
            module_impl.hpp
            module_info.cpp
            module_info.hpp
            platform.cpp
            platform.hpp
            process_context.cpp
            process_context.hpp
            progress_dialog.hpp
            regex_lists_settings.cpp
            regex_lists_settings.hpp
            source_file.cpp
            source_file.hpp
            source_file_vector_model.cpp
            source_file_vector_model.hpp
            source_model.cpp
            source_model.hpp
            source_position_breakpoint.cpp
            source_position_breakpoint.hpp
            source_position.cpp
            source_position.hpp
            source_position_info.cpp
            source_position_info.hpp
            source_tree.cpp
            source_tree.hpp
            source_tree_builder.cpp
            source_tree_builder.hpp
            source_tree_model.hpp
            stack_frame.cpp
            stack_frame.hpp
            stack_frame_info.cpp
            stack_frame_info.hpp
            stop_reason.cpp
            stop_reason.hpp
            target.cpp
            target.hpp
            target_impl.cpp
            target_impl.hpp
            term_settings.cpp
            term_settings.hpp
            thread.cpp
            thread.hpp
            thread_info.cpp
            thread_info.hpp
            thread_list.cpp
            thread_list.hpp
            thread_list_info.cpp
            thread_list_info.hpp
            thread_utils.hpp
            watchpoint.cpp
            watchpoint.hpp
            watch_list.cpp
            watch_list.hpp
            watch_list_impl.hpp
            watch_model.cpp
            watch_model.hpp
           )

target_precompile_headers(cxxdbg-dbg PRIVATE import.hpp)

target_include_directories(cxxdbg-dbg PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

install_cxxdbg_library(cxxdbg-dbg)

target_link_libraries(cxxdbg-dbg PUBLIC cxxdbg-app cxxdbg-log cxxdbg-dbg-cli
                                     cxxdbg-async cxxdbg-dbg-execstub cxxdbg-util cxxdbg-dbg-core)
target_link_libraries(cxxdbg-dbg PUBLIC Boost::program_options)
target_link_libraries(cxxdbg-dbg PUBLIC ranges)
if("${CXXDBG_NEED_LINK_STDCXXFS}")
    target_link_libraries(cxxdbg-dbg PUBLIC stdc++fs)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(cxxdbg-dbg PUBLIC dl)
endif()

add_subdirectory(core)
add_subdirectory(dapsrv)
add_subdirectory(test)
add_subdirectory(appcore)
