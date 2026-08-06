# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# library for exchaning data betwee exec stub and cxxdbg
add_library(cxxdbg-dbg-execstub ${LIBRARY_TYPE}
            exec_stub_client.cpp
            exec_stub_client.hpp
            exec_stub_server.cpp
            exec_stub_server.hpp
            exec_stub_struct.cpp
            exec_stub_struct.hpp
           )

install_cxxdbg_library(cxxdbg-dbg-execstub)

target_link_libraries(cxxdbg-dbg-execstub PUBLIC Boost::interprocess
                                             cxxdbg-boost-process
                                             Boost::date_time
                                             Boost::system
                                             Boost::boost)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(cxxdbg-dbg-execstub PUBLIC rt dl)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(cxxdbg-dbg-execstub PUBLIC ws2_32)
endif()

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(cxxdbg-dbg-execstub PUBLIC -pthread)
endif()


add_executable(cxxdbg-exec-stub exec_stub.cpp)

# set relative rpath for cxxdbg-exec-stub
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg-exec-stub PROPERTY INSTALL_RPATH "@executable_path/../Frameworks")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set_property(TARGET cxxdbg-exec-stub PROPERTY INSTALL_RPATH "$ORIGIN/../lib/cxxdbg")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg-exec-stub PROPERTY RUNTIME_OUTPUT_DIRECTORY
                    "${CMAKE_CURRENT_BINARY_DIR}/../../cxxdbg.app/Contents/bin")
endif()

# linking with util and license library
target_link_libraries(cxxdbg-exec-stub cxxdbg-dbg-execstub cxxdbg-util cxxdbg-log)

# linking with boost program options
target_link_libraries(cxxdbg-exec-stub Boost::program_options)

# cxxdbg-exec-stub will be installed together with cxxdbg.app bundle
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    install(TARGETS cxxdbg-exec-stub
            RUNTIME DESTINATION "bin"
            COMPONENT cxxdbg)
endif()


add_subdirectory(test)
