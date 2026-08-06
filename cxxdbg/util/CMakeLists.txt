# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Formatters library
add_library(cxxdbg-util ${LIBRARY_TYPE}
            boost_asio_io_pool.cpp
            boost_asio_io_pool.hpp
            command_line_parser.cpp
            command_line_parser.hpp
            convert.hpp
            execute.hpp
            model_printer.hpp
            paths.cpp
            paths.hpp
            print.cpp
            print.hpp
            signals.hpp
            string_vector.cpp
            string_vector.hpp
            template.cpp
            template.hpp
            touch.cpp
            touch.hpp
            treewalk.hpp
            version.hpp
            version.cpp)

target_link_libraries(cxxdbg-util PUBLIC cxxdbg-log ranges)
target_link_libraries(cxxdbg-util PUBLIC cxxdbg-boost-process
                                      Boost::filesystem
                                      Boost::system
                                      Boost::thread
                                      Boost::boost)
if("${CXXDBG_NEED_LINK_STDCXXFS}")
    target_link_libraries(cxxdbg-util PUBLIC stdc++fs)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(cxxdbg-util PUBLIC ws2_32 bcrypt)
endif()

install_cxxdbg_library(cxxdbg-util)

add_subdirectory(test)

