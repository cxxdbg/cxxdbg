# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

add_executable(cxxdbg-cli main.cpp)

target_link_libraries(cxxdbg-cli PRIVATE cxxdbg-app cxxdbg-log cxxdbg-dbg-core)

# Enable hooks in driver
target_compile_definitions(cxxdbg-cli PRIVATE LLDB_DRIVER_HOOKS=1)

target_link_libraries(cxxdbg-cli PRIVATE llvm-includes)
target_link_libraries(cxxdbg-cli PRIVATE cxxdbg-llvm)

# Put cxxdbg-cli utility into the CxxDbg.app/Contents/bin subdirectory on MacOS.
# We don't need install rules for MacOS because it will be signed and installed
# together with the bundle.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg-cli PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../CxxDbg.app/Contents/bin")
else()
    install(TARGETS cxxdbg-cli DESTINATION "bin" COMPONENT "cxxdbg")
endif()

