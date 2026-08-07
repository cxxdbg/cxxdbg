# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# core library dependent from llvm
add_library(cxxdbg-dbg-corellvm OBJECT
            array_type_impl.cpp
            array_type_impl.hpp
            code_breakpoint.cpp
            code_breakpoint.hpp
            breakpoint.hpp
            breakpoint_id.hpp
            breakpoint_base_impl.hpp
            breakpoint_location.cpp
            breakpoint_location.hpp
            builtin_type_impl.cpp
            builtin_type_impl.hpp
            code_model.hpp
            code_position.cpp
            code_position.hpp
            compile_unit.cpp
            compile_unit.hpp
            debugger_base.cpp
            debugger_base.hpp
            disassembler.hpp
            enum_type_impl.cpp
            enum_type_impl.hpp
            function_debug_object_impl.cpp
            function_debug_object_impl.hpp
            function_type_impl.cpp
            function_type_impl.hpp
            global_variable.cpp
            global_variable.hpp
            instruction.cpp
            instruction.hpp
            lldb_debugger_ext_base.cpp
            lldb_debugger_ext_base.hpp
            lldb_disassembler.cpp
            lldb_disassembler.hpp
            lldb_listener_event_queue.cpp
            lldb_listener_event_queue.hpp
            lldb_memory_storage.cpp
            lldb_memory_storage.hpp
            lldb_pointer_type_impl.cpp
            lldb_pointer_type_impl.hpp
            lldb_value_storage.cpp
            lldb_value_storage.hpp
            log.hpp
            import.hpp
            memory_disassembler.cpp
            memory_disassembler.hpp
            mem_ptr_type_impl.cpp
            mem_ptr_type_impl.hpp
            module.cpp
            module.hpp
            regex.cpp
            regex.hpp
            symbol.cpp
            symbol.hpp
            symbol_impl.cpp
            symbol_impl.hpp
            pointer_type_impl.cpp
            pointer_type_impl.hpp
            record_type_impl.cpp
            record_type_impl.hpp
            registers_base.cpp
            registers_base.hpp
            signals.hpp
            source_position.cpp
            source_position.hpp
            stack_frame_base.cpp
            stack_frame_base.hpp
            stop_reason.hpp
            target_base.cpp
            target_base.hpp
            thread_base.cpp
            thread_base.hpp
            typedef_type_impl.cpp
            typedef_type_impl.hpp
            type_utils.cpp
            type_utils.hpp
            utils.cpp
            utils.hpp
            type_impl.cpp
            type_impl.hpp
            variable.cpp
            variable.hpp
            vector_type_impl.cpp
            vector_type_impl.hpp
            watchpoint.cpp
            watchpoint.hpp
           )

target_precompile_headers(cxxdbg-dbg-corellvm PRIVATE "import.hpp")

if(CXXDBG_ENABLE_SHARED AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_compile_options(cxxdbg-dbg-corellvm PRIVATE -fPIC)
endif()

target_link_libraries(cxxdbg-dbg-corellvm PUBLIC llvm-includes)

# linking with async and format libraries
target_link_libraries(cxxdbg-dbg-corellvm PUBLIC cxxdbg-dbg-fmtcore cxxdbg-dbg-fmtcommon cxxdbg-async cxxdbg-proc cxxdbg-util cxxdbg-log)
target_link_libraries(cxxdbg-dbg-corellvm PUBLIC ranges)

# linking with boost
target_link_libraries(cxxdbg-dbg-corellvm PUBLIC Boost::dll)


# core library
add_library(cxxdbg-dbg-core ${LIBRARY_TYPE}
            debugger.cpp
            debugger.hpp
            format.cpp
            format.hpp
            import.hpp
            lldb_debugger_ext.cpp
            lldb_debugger_ext.hpp
            regex_lists.cpp
            regex_lists.hpp
            regex_lists_defaults.cpp
            regex_lists_defaults.hpp
            registers.cpp
            registers.hpp
            stack_frame.cpp
            stack_frame.hpp
            target.cpp
            target.hpp
            thread.cpp
            thread.hpp
            tn_code_model.hpp
            tn_function_name_parser.cpp
            tn_function_name_parser.hpp
            )

target_precompile_headers(cxxdbg-dbg-core PRIVATE "import.hpp")

target_link_libraries(cxxdbg-dbg-core PUBLIC cxxdbg-llvm cxxtn cxxdbg-dbg-fmt)

install_cxxdbg_library(cxxdbg-dbg-core)

add_subdirectory(test)

