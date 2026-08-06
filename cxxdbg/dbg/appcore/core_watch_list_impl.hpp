// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_watch_list_impl.hpp
/// Contains definition of core_watch_list_impl class.

#pragma once

#include "watch_list_server.hpp"
#include "cxxdbg/async/forward.hpp"
#include "cxxdbg/dbg/forward.hpp"
#include "cxxdbg/async/async_execution_queue.hpp"
#include "dbgfmt/context.hpp"
#include "cxxdbg/dbg/watch_list_impl.hpp"


namespace cxxdbg::dbg::core {
    class target;
}


namespace cxxdbg::dbg::appcore {


/// \class core_watch_list_impl
/// Base class for all implementation of watch list for core library
class core_watch_list_impl: public dbg::watch_list_server, virtual public dbg::watch_list_impl {
public:
    /// Constructors watch list with specified reference
    /// to core execution queue and reference to target object
    core_watch_list_impl(async::execution_queue & exec_queue, core::target & t);

    /// Destructor, destroys object
    ~core_watch_list_impl() override = default;

    /// Connects to tree updated signal. The signal is emitted after
    /// watch tree has been updated
    signal_connection connect_tree_updated(const tree_updated_handler & handl) override;

    /// Exapnds tree node with specified id
    void expand_node(node_id id, const node_handler & handl) override;

    /// Collapses tree node with specified id
    void collapse_node(node_id id, const node_handler & handl) override;

    /// Sets format options. Calls completion handler after formatting tree with new options.
    void set_fmt_opts(const dbgfmt::format_options & fmt_opts, const tree_updated_handler & handl) override;

    /// Emits tree updated signal to listeners. Should be called in main thread
    void emit_tree_updated(const tree_info & tinfo);

protected:
    /// Returns reference to async execution queue for core thread
    async::execution_queue & exec_queue();

    /// Returns reference to core target
    core::target & targ() const { return targ_; }

private:
    async::execution_queue & exec_queue_;           ///< Reference to async exection queue for core commands
    core::target & targ_;                           ///< Reference to core target
    signal<tree_updated_func> tree_updated_;        ///< Tree updated signal
};


}


