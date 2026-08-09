# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

add_library(cxxdbg-dbg-dapsrv ${LIBRARY_TYPE}
            asio_event_queue.hpp
            child_process.hpp
            client_term_debug_child_process.cpp
            client_term_debug_child_process.hpp
            client_term_exec_stub_launcher.cpp
            client_term_exec_stub_launcher.hpp
            client_term_process_launcher.cpp
            client_term_process_launcher.hpp
            debug_child_process.cpp
            debug_child_process.hpp
            import.hpp
            nodebug_child_process.cpp
            nodebug_child_process.hpp
            server_impl.cpp
            server_impl.hpp
            stack_frame_compare.hpp
            variable_reference.cpp
            variable_reference.hpp)

target_precompile_headers(cxxdbg-dbg-dapsrv PRIVATE "import.hpp")

target_link_libraries(cxxdbg-dbg-dapsrv PUBLIC cxxdbg-app cxxdap cxxdbg-dbg-core cxxdbg-dbg-execstub cxxdbg-log)

install_cxxdbg_library(cxxdbg-dbg-dapsrv)

add_executable(cxxdbg-dap-server main.cpp)
target_link_libraries(cxxdbg-dap-server PUBLIC cxxdbg-dbg-dapsrv)
target_link_libraries(cxxdbg-dap-server PUBLIC Boost::program_options Boost::iostreams)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg-dap-server PROPERTY INSTALL_RPATH "@executable_path/../Frameworks")
else()
    set_property(TARGET cxxdbg-dap-server PROPERTY INSTALL_RPATH "$ORIGIN/../lib")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg-dap-server PROPERTY RUNTIME_OUTPUT_DIRECTORY
                "${CMAKE_CURRENT_BINARY_DIR}/../../../CxxDbg.app/Contents/bin")
endif()

# dap server will be installed together with CxxDbg.app bundle
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    install(TARGETS cxxdbg-dap-server RUNTIME DESTINATION "bin" COMPONENT "cxxdbg")
endif()
