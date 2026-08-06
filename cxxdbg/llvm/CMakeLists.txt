# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# cxxdbg-llvm is the library that contains all required LLVM libraries. We need
# move all llvm related code to separate library becuase we build
# LLVM with -fdefault-visibility=hidden flag. This is required to
# be able to use LLVM with mesa driver.

# We add lldb driver code to cxxdbg-llvm library to be able to use
# it in cxxdbg-cli console application

add_library(cxxdbg-llvm ${LIBRARY_TYPE}
            "${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb/tools/driver/Driver.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb/tools/driver/Platform.cpp")

if("${CXXDBG_BUILD_LLVM}")
    add_dependencies(cxxdbg-llvm LLDBOptionsTableGen)
endif()

# Enable library variant in driver code
target_compile_definitions(cxxdbg-llvm PRIVATE LLDB_DRIVER_LIBRARY=1)

# Include path for Options.inc
if("${LLDB_ROOT}" STREQUAL "${LLVM_ROOT}")
    target_include_directories(cxxdbg-llvm PRIVATE "${LLVM_ROOT}/tools/lldb/tools/driver")
else()
    target_include_directories(cxxdbg-llvm PRIVATE "${LLDB_ROOT}/lldb/tools/driver")
endif()

target_link_libraries(cxxdbg-llvm PUBLIC cxxdbg-dbg-corellvm)

install_cxxdbg_library(cxxdbg-llvm)

# linking with llvm libraries
target_link_libraries(cxxdbg-llvm PRIVATE cxxdbg-llvm-libraries)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(cxxdbg-llvm PRIVATE "-Wl,--no-undefined")
endif()

