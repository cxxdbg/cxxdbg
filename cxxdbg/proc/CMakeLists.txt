# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Process library
add_library(cxxdbg-proc ${LIBRARY_TYPE}
            bp_child_monitor.cpp
            bp_child_monitor.hpp
            bp_io_child_monitor.hpp
            bp_io_launcher.cpp
            bp_io_launcher.hpp
            bp_launcher.cpp
            bp_launcher.hpp
            bp_terminal_launcher.hpp
            child_io_monitor.hpp
            launch_parameters.cpp
            launch_parameters.hpp
            launcher.hpp
            monitor.hpp
            log.hpp
            system_monitor.hpp
            )


if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
target_sources(cxxdbg-proc PRIVATE
               win32_monitor.cpp
               win32_monitor.hpp)
else()
    target_sources(cxxdbg-proc PRIVATE
                   async_pty.cpp
                   async_pty.hpp
                   bp_pty_launcher.cpp
                   bp_pty_launcher.hpp
                   child_pty.hpp
                   posix_monitor.cpp
                   posix_monitor.hpp)
endif()


target_link_libraries(cxxdbg-proc PUBLIC cxxdbg-log)
target_link_libraries(cxxdbg-proc PUBLIC ranges)
target_link_libraries(cxxdbg-proc PUBLIC cxxdbg-boost-process Boost::system Boost::thread Boost::boost)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(cxxdbg-proc PUBLIC ws2_32)
endif()

install_cxxdbg_library(cxxdbg-proc)

add_subdirectory(test)

