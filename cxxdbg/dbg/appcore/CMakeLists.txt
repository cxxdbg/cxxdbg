# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

add_library(cxxdbg-dbg-appcore ${LIBRARY_TYPE}
            appcore.cpp
            appcore.hpp
            core_debugger_impl.cpp
            core_debugger_impl.hpp
            core_locals_list_impl.cpp
            core_locals_list_impl.hpp
            core_module_impl.cpp
            core_module_impl.hpp
            core_target_impl.cpp
            core_target_impl.hpp
            core_watch_list_impl.cpp
            core_watch_list_impl.hpp
            core_custom_watch_list_impl.cpp
            core_custom_watch_list_impl.hpp
            registers_list_impl.cpp
            registers_list_impl.hpp
            tree_node_key.cpp
            tree_node_key.hpp
            utils.cpp
            utils.hpp
            watch_list_server_tree_node.hpp
            watch_list_server.cpp
            watch_list_server.hpp
        )

install_cxxdbg_library(cxxdbg-dbg-appcore)

# linking with lldb
target_link_libraries(cxxdbg-dbg-appcore PUBLIC
                      cxxdbg-dbg
                      cxxdbg-async
                      cxxdbg-dbg-core
                      cxxdbg-dbg-fmt
                      cxxdbg-dbg-fmtcore
                     )


add_subdirectory(test)
