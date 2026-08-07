# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# including version information
include("${CMAKE_CURRENT_SOURCE_DIR}/version.cmake")

message(STATUS "Version suffix is set to ${CXXDBG_VERSION_SUFFIX}")

# configuring version_config.hpp header
configure_file("version_config.hpp.cmake.in" "version_config.hpp" @ONLY)


add_library(cxxdbg-app ${LIBRARY_TYPE}
            application_log.cpp
            application_log.hpp
            async_data_fetcher.cpp
            async_data_fetcher.hpp
            async_data_hex_text_model.hpp
            async_data_source.hpp
            basic_tree_model.hpp
            cstring_vector_model.cpp
            cstring_vector_model.hpp
            data_hex_text_model.cpp
            data_hex_text_model.hpp
            document.hpp
            document_list.cpp
            document_list.hpp
            file_document.cpp
            file_document.hpp
            flat_tree_model.hpp
            forward.hpp
            json_settings_storage.cpp
            json_settings_storage.hpp
            memory_settings_storage.cpp
            memory_settings_storage.hpp
            recent_file_list.cpp
            recent_file_list.hpp
            settings_storage.cpp
            settings_storage.hpp
            signals.cpp
            signals.hpp
            simple_tree_view_model.cpp
            simple_tree_view_model.hpp
            sortable_proxy_tree_view_model.cpp
            sortable_proxy_tree_view_model.hpp
            terminal.hpp
            text_document.hpp
            text_file_document.hpp
            tree_model.hpp
            tree_view_model.cpp
            tree_view_model.hpp
            tree_view_model_adapter.hpp
            vector_model.hpp
            vector_ptr_model.hpp
            vector_ptr_tree_model.hpp
            version.hpp
            )

target_include_directories(cxxdbg-app PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

install_cxxdbg_library(cxxdbg-app)

target_link_libraries(cxxdbg-app PUBLIC tmvc cxxdbg-log cxxdbg-dbg-cli cxxdbg-dbg-fmt cxxdbg-async cxxdbg-dbg-execstub cxxdbg-util)
target_link_libraries(cxxdbg-app PUBLIC Boost::program_options)
find_package(nlohmann_json REQUIRED)
target_link_libraries(cxxdbg-app PUBLIC ranges nlohmann_json::nlohmann_json)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(cxxdbg-app PUBLIC dl)
endif()

add_subdirectory(test)
