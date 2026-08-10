# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Application test
add_executable(apptest
               async_data_fetcher_test.cpp
               basic_tree_model_test.cpp
               data_hex_text_model_test.cpp
               document_list_test.cpp
               flat_tree_model_test.cpp
               settings_storage_test.cpp
               simple_tree_view_model_test.cpp
               sortable_proxy_tree_view_model_test.cpp
               test.cpp
               vector_ptr_tree_model_test.cpp
               vector_ro_tree_view_model_test.cpp)

target_link_libraries(apptest PRIVATE cxxdbg-app)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(apptest PRIVATE rt)
endif()

add_test(NAME apptest COMMAND apptest)
