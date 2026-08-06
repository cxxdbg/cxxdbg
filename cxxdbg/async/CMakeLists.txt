# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# async unitilies library
add_library(cxxdbg-async ${LIBRARY_TYPE}
            async_execution_queue.cpp
            async_execution_queue.hpp
            event_queue.hpp
            forward.hpp
            result.cpp
            result.hpp
            simple_event_queue.cpp
            simple_event_queue.hpp
           )

target_link_libraries(cxxdbg-async PUBLIC Boost::system Boost::thread Boost::boost)

install_cxxdbg_library(cxxdbg-async)

add_subdirectory(test)
