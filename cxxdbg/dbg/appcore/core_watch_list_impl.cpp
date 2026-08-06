// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_watch_list_impl.cpp
/// Contains implementation of core_watch_list_impl class.

#include "core_watch_list_impl.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "dbgfmt/fixed_fmt_result.hpp"


namespace cxxdbg::dbg::appcore {


core_watch_list_impl::core_watch_list_impl(async::execution_queue & exec_queue, core::target & t):
exec_queue_{exec_queue}, targ_{t} {
}


signal_connection core_watch_list_impl::connect_tree_updated(const tree_updated_handler & handl) {
    return tree_updated_.connect(handl);
}


void core_watch_list_impl::expand_node(node_id id, const node_handler & handl) {

    exec_queue_.add_command_eh(
        [this, handl](const tree_info::node & node) {
            handl(node);
        },
        [this, id]() -> tree_info::node {
            return expand_node_async(id);
        }
    );
}


void core_watch_list_impl::collapse_node(dbg::watch_list_impl::node_id id, const node_handler & handl) {
    exec_queue_.add_command_eh(
        [this, handl](const tree_info::node & node) {
            handl(node);
        },
        [this, id]() -> tree_info::node {
            return collapse_node_async(id);
        }
    );
}


void core_watch_list_impl::set_fmt_opts(const dbgfmt::format_options & fmt_opts, const tree_updated_handler & handl) {
    exec_queue_.add_command_eh(
        [this, handl](const tree_info & tree) {
            handl(tree);
        },
        [this, fmt_opts]() -> tree_info {
            targ_.set_fmt_opts(fmt_opts);
            return make_values_tree();
        }
    );
}


void core_watch_list_impl::emit_tree_updated(const tree_info & tinfo) {
    tree_updated_(tinfo);
}


async::execution_queue & core_watch_list_impl::exec_queue() {
    return exec_queue_;
}


}
