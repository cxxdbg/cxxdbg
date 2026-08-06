// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_custom_watch_list_impl.hpp
/// Contains definition of the core_custom_watch_list_impl class.

#pragma once

#include "core_watch_list_impl.hpp"
#include "cxxdbg/dbg/custom_watch_list_impl.hpp"
#include "dbgfmt/fmt_result.hpp"


namespace cxxdbg::dbg {

namespace core {
    class target;
    class variable;
}


namespace appcore {


/// Watch list implementation which displays list of values specified
/// by user
class core_custom_watch_list_impl:
        public core_watch_list_impl,
        virtual public dbg::custom_watch_list_impl {
public:
    /// Constructor, makes custom watch list with specified reference to
    /// core thread execution queue, reference to core target, and
    /// reference to formatting context
    core_custom_watch_list_impl(async::execution_queue & exec_queue, core::target & targ);

    /// Destructor, destroys object
    ~core_custom_watch_list_impl() override = default;

    /// Adds expression to watch list
    void add_watch(const std::wstring & expr, const node_handler & handl) override;

    /// Remvoes expression with specified index from watch list
    void remove_watch(node_id id) override;

    /// Sets expression for watch with spefified ID of root node
    void set_watch(node_id id, const std::wstring & expr, const node_handler & handl) override;

    /// Clears all values in watch list. Called when process is killed.
    /// Called in core thread
    void clear();

private:
    struct custom_watch_info {
        watch_id id;
        std::wstring expr;
    };

    /// Returns map of (id, value) in watch list
    std::map<watch_id, dbgfmt::named_fmt_result> get_variables() override;

    /// Reads list of local variables for current frame. Returns empty vector
    /// if current state is not stopped
    std::vector<const core::variable*> read_locals() const;

    /// Formats watch expression using specified list of local variables to search
    /// variable with name equal to expression first
    dbgfmt::named_fmt_result format_expr(const std::wstring & wexpr,
                                      const std::vector<const core::variable*> & vars) const;

    /// Formats watch expression. Reads list of local variables to search for local variable
    /// before formatting
    dbgfmt::named_fmt_result format_expr(const std::wstring & wexpr) const;

    std::vector<custom_watch_info> watches_;    ///< Vector of watch values
    watch_id next_id_{0};                       ///< next ID for new watch
};


} }


