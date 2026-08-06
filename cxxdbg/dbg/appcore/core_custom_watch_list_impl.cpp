// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file core_custom_watch_list_impl.cpp
/// Contains implementation of the core_custom_watch_list_impl class.

#include "core_custom_watch_list_impl.hpp"
#include "dbgfmt/fixed_fmt_result.hpp"
#include "cxxdbg/dbg/core/target.hpp"
#include "cxxdbg/dbg/core/variable.hpp"
#include "dbgfmt/backend/format_error.hpp"


namespace cxxdbg::dbg::appcore {


core_custom_watch_list_impl::core_custom_watch_list_impl(async::execution_queue & exec_queue, core::target & targ):
core_watch_list_impl(exec_queue, targ) {
}


void core_custom_watch_list_impl::add_watch(const std::wstring & expr, const node_handler & handl) {
    exec_queue().add_command_eh(
    [this, handl](const tree_info::node & node) {
        handl(node);
    },
    [this, expr] {
        auto wid = next_id_;
        ++next_id_;

        // adding watch to list of watches
        watches_.push_back({wid, expr});

        // adding watch into watch server and formatting it
        return watch_list_server::add_watch(wid, format_expr(expr));
    });
}


void core_custom_watch_list_impl::remove_watch(node_id id) {
    exec_queue().add_command_eh(
    [this]() {},
    [this, id] {
        watch_id wid = root_node_watch_id(id);
        auto it = watches_.begin();
        for (auto end = watches_.end(); it != end; ++it) {
            if (it->id == wid) {
                break;
            }
        }

        assert(it != watches_.end() && "can't find watch with");
        watches_.erase(it);
    });
}


void core_custom_watch_list_impl::set_watch(node_id id, const std::wstring & expr, const node_handler & handl) {
    exec_queue().add_command_eh(
    [this, handl](const tree_info::node & node) {
        handl(node);
    },
    [this, id, expr] {
        watch_id wid = root_node_watch_id(id);
        auto it = watches_.begin();
        for (auto end = watches_.end(); it != end; ++it) {
            if (it->id == wid) {
                break;
            }
        }

        assert(it != watches_.end() && "can't find watch with");
        it->expr = expr;

        return make_watch_info(wid, format_expr(expr));
    });
}


void core_custom_watch_list_impl::clear() {
    // clearing values tree
    make_values_tree();
}

std::map<core_watch_list_impl::watch_id, dbgfmt::named_fmt_result>
core_custom_watch_list_impl::get_variables() {
    // this function is being executed in core thread

    auto vars = read_locals();

    std::map<watch_id, dbgfmt::named_fmt_result> res;
    for (auto && w : watches_) {
        res.insert({w.id, format_expr(w.expr, vars)});
    }

    return res;
}


std::vector<const core::variable*> core_custom_watch_list_impl::read_locals() const {
    if (targ().state() != core::target::state_t::stopped) {
        return {};
    }

    auto frame = targ().thread_at(targ().current_thread_index()).selected_frame();
    if (!frame.is_valid()) {
        return {};
    }

    return frame.read_locals();
}


dbgfmt::named_fmt_result
core_custom_watch_list_impl::format_expr(const std::wstring & wexpr,
                                         const std::vector<const core::variable*> & vars) const {

    if (targ().state() != core::target::state_t::stopped) {
        // returning null value if not in stopped state
        return {wexpr, {}, dbgfmt::make_fmt_result<dbgfmt::fixed_fmt_result>(L"", L"", dbgfmt::backend::source_position{}, dbgfmt::backend::source_position{})};
    }

    // TODO: do we need real unicode support in expressions?
    std::string expr{wexpr.begin(), wexpr.end()};

    // first trying find expression in the list of local variables.
    // lldb does not correctly preserve all type info when computing values
    // of expressions so local variables are displayed better
    auto var_it = std::find_if(vars.begin(), vars.end(), [&expr](const auto & v) {
        return v->name() == expr;
    });

    if (var_it != vars.end()) {
        auto frame = targ().thread_at(targ().current_thread_index()).selected_frame();
        return {wexpr, {}, targ().format_value(frame.read_var_value(*var_it))};
    } else {
        dbgfmt::fmt_result_ref eval_res;
        try {
            eval_res = targ().format_value(targ().eval_expr(expr));
        }
        catch(dbgfmt::backend::format_error & err) {
            eval_res = dbgfmt::make_error_fmt_result(err);
        }

        return {wexpr, {}, eval_res};
    }
}


dbgfmt::named_fmt_result core_custom_watch_list_impl::format_expr(const std::wstring & wexpr) const {
    return format_expr(wexpr, read_locals());
}


}
