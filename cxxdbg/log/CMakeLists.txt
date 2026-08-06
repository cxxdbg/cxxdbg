# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

add_library(cxxdbg-log ${LIBRARY_TYPE}
            log.hpp
            log_init.cpp
            log_init.hpp)

target_link_libraries(cxxdbg-log PUBLIC Boost::program_options Boost::log Boost::boost)

if("${CXXDBG_NEED_LINK_STDCXXFS}")
    target_link_libraries(cxxdbg-log PUBLIC stdc++fs)
endif()

# we need pass correct compile flags to all log dependent code if
# shared boost log library is used
string(TOUPPER "${CMAKE_BUILD_TYPE}" bt_upper)
get_property(loc TARGET Boost::log PROPERTY IMPORTED_LOCATION_${bt_upper})
if ("${loc}" STREQUAL "")
    get_property(loc TARGET Boost::log PROPERTY IMPORTED_LOCATION)
endif()

get_filename_component(ext "${loc}" EXT)
if("${ext}" MATCHES "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    target_compile_definitions(cxxdbg-log PUBLIC -DBOOST_LOG_DYN_LINK=1)
else()
    target_compile_definitions(cxxdbg-log PUBLIC -DBOOST_LOG_NO_DYN_LINK=1)
endif()

# we have to define this macro on windows because boost log library uses
# name versioning dependent on target windows platform, and boost builds by
# default for Windows XP
#if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
#    target_compile_definitions(cxxdbg-log PUBLIC "-DBOOST_USE_WINAPI_VERSION=0x0501")
#endif()

install_cxxdbg_library(cxxdbg-log)
