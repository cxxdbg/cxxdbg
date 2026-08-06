// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target.hpp
/// Contains definition of the target class

#pragma once

#include "dbgfmt/format_options.hpp"
#include "target_base.hpp"
#include "thread.hpp"
#include "dbgfmt/context.hpp"


namespace cxxdbg::dbg::core {

class debugger;


/// Represents target in debugger
class target: public target_base {
    friend class debugger;

public:
    /// Constructs target by loading executable with specified path
    target(debugger_base & dbg, const std::filesystem::path & exe_path):
    target_base(dbg, exe_path), fmt_ctx_{*this, *this} {
    }

    /// Consturcts target by attaching to process with specified pid
    target(debugger_base & dbg, unsigned long pid):
    target_base(dbg, pid), fmt_ctx_{*this, *this} {
    }

    /// Constructs target by attaching to process with specified name
    target(debugger_base & dbg, const std::string & name):
    target_base(dbg, name), fmt_ctx_{*this, *this} {
    }

    /// Formats value
    dbgfmt::fmt_result_ref format_value(const std::shared_ptr<dbgfmt::backend::value_impl> & v) {
        return fmt_ctx_.format(dbgfmt::value{v});
    }

    /// Sets format options for formatting values
    void set_fmt_opts(const dbgfmt::format_options & opts) {
        fmt_ctx_.set_fmt_opts(opts);
    }


    //////////////////////////////////////////////////
    // Stepping

    /// Continues execution of target. May be called in stopped state.
    /// After calling, the state of target becomes running.
    void run() {
        step_thread_id_ = 0;
        target_base::run();
    }

    /// Performs "step into" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_into(unsigned long thread_id,
                   bool avoid_nodebug,
                   const std::string & target,
                   const std::string & avoid_regex,
                   const std::string & step_through_regex) {
        step_thread_id_ = thread_id;
        target_base::step_into(thread_id, avoid_nodebug, target, avoid_regex, step_through_regex);
    }

    /// Performs "step over" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_over(unsigned long thread_id, const std::string & step_through_regex) {
        step_thread_id_ = thread_id;
        target_base::step_over(thread_id, step_through_regex);
    }

    /// Performs "step out" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_out(unsigned long thread_id, bool avoid_nodebug, const std::string & step_through_regex) {
        step_thread_id_ = thread_id;
        target_base::step_out(thread_id, avoid_nodebug, step_through_regex);
    }

    /// Performs "step instruction into" action on target. Can be called
    /// only in 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_into(unsigned long thread_id) {
        step_thread_id_ = thread_id;
        target_base::step_inst_into(thread_id);
    }

    /// Performs "step instruction over" action on target. Can be called
    /// only in 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_over(unsigned long thread_id) {
        step_thread_id_ = thread_id;
        target_base::step_inst_over(thread_id);
    }

    /// Performs "step until" action on target. Can be called only
    /// in 'stopped' state. After calling, the state of target becomes
    /// 'running'.
    void step_until(unsigned long thread_id, unsigned int line) {
        step_thread_id_ = thread_id;
        target_base::step_until(thread_id, line);
    }


    //////////////////////////////////////////////////
    // Threads

    /// Returns thread at specified index
    thread thread_at(size_t idx);

    /// Returns range of all threads in target
    auto threads() {
        auto indexes = std::ranges::views::iota(size_t(), threads_size());

        auto fn = [this](size_t idx) {
            return thread_at(idx);
        };

        return indexes | std::ranges::views::transform(fn);
    }

    /// Returns stop reason for target in stopped state
    stop_reason_t stop_reason();

    /// Sets active thread based on stop reason
    void select_active_thread_by_stop_reason();

    /// Returns active thread for current stop reason. Stop reason thread must exists.
    thread stop_reason_thread();

    /// Returns active thread index for current stop reason, or SIZE_MAX if stop reason
    /// thread does not exist
    size_t stop_reason_thread_index();

    /// Returns thread index by id
    size_t thread_index_by_id(unsigned long id);

    /// Returns thread by id
    thread thread_by_id(unsigned long id);


    //////////////////////////////////////////////////
    // Expressions

    /// Evaluates expression and returns variable containing result of evaluation
    std::shared_ptr<dbgfmt::backend::value_impl> eval_expr(const std::string & expr);


    /// Makes temporary value of specified type
    std::shared_ptr<dbgfmt::backend::value_impl> make_temp_val(const dbgfmt::backend::type_impl * type) override;

private:
    /// Constructs dummy target
    target(debugger_base & dbg): target_base(dbg), fmt_ctx_{*this, *this} {}

    /// Returns frame object for current selected frame
    /// TODO: refactor find_var and remove this function.
    stack_frame_base selected_frame_base() override;

    dbgfmt::context fmt_ctx_;          ///< Formatting context;

    /// ID of thread for which last step operation was performed
    unsigned long step_thread_id_ = 0;
};


}
