# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--no-undefined")
endif()

add_subdirectory(mock)
add_subdirectory(boost_process)
add_subdirectory(util)
add_subdirectory(async)
add_subdirectory(proc)
add_subdirectory(log)
add_subdirectory(llvm)
add_subdirectory(app)
add_subdirectory(gui)
add_subdirectory(exec_stub)
add_subdirectory(cli)
add_subdirectory(dbg)

if("${CXXDBG_ENABLE_CLI}")
    add_subdirectory(cxxdbg-cli)
endif()
