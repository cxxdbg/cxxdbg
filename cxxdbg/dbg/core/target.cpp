// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target.cpp
/// Contains implementation of the target class.

#include "target.hpp"
#include "log.hpp"
#include "dbgfmt/temp_value_impl.hpp"


namespace cxxdbg::dbg::core {


thread target::thread_at(size_t idx) {
    return thread{*this, idx};
}


stop_reason_t target::stop_reason() {
    auto idx = stop_reason_thread_index();
    if (idx == SIZE_MAX) {
        return stop_reason_t::unknown;
    }

    return thread_at(idx).stop_reason();
}


void target::select_active_thread_by_stop_reason() {
    auto idx = stop_reason_thread_index();
    if (idx == SIZE_MAX) {
        return;
    }

    set_current_thread_index(idx);
}


thread target::stop_reason_thread() {
    auto idx = stop_reason_thread_index();
    if (idx == SIZE_MAX) {
        auto msg = "can't get stop reason thread";
        CXXDBG_CORE_LOG_ERROR << msg;
        assert(false && "can't get stop reason thread");
        throw std::runtime_error{msg};
    }

    return thread_at(idx);
}


size_t target::stop_reason_thread_index() {
    // first checking if we have thread ID of previous step operation and it has stop reason
    if (step_thread_id_ != 0) {
        auto idx = thread_index_by_id(step_thread_id_);
        if (idx != SIZE_MAX) {
            auto thrd = thread_at(idx);
            if (thrd.stop_reason() != stop_reason_t::unknown) {
                return idx;
            }
        }
    }

    // looking for thread with any stop reason over than unknown
    size_t idx = 0;
    for (auto thrd : threads()) {
        if (thrd.stop_reason() != stop_reason_t::unknown) {
            return idx;
        }

        ++idx;
    }

    CXXDBG_CORE_LOG_DEBUG << "stop reason thread not found";
    return SIZE_MAX;
}


size_t target::thread_index_by_id(unsigned long id) {
    size_t idx = 0;
    for (auto thrd : threads()) {
        if (thrd.id() == id) {
            return idx;
        }
        ++idx;
    }

    return SIZE_MAX;
}


std::shared_ptr<dbgfmt::backend::value_impl> target::eval_expr(const std::string & expr) {
    return thread_at(current_thread_index()).selected_frame().eval_expr(expr);
}


std::shared_ptr<dbgfmt::backend::value_impl> target::make_temp_val(const dbgfmt::backend::type_impl * type) {
    return std::make_shared<dbgfmt::temp_value_impl>(*this, type, memory(), endianness());
}


thread target::thread_by_id(unsigned long id) {
    auto idx = thread_index_by_id(id);
    assert(idx != SIZE_MAX && "can't find thread with ID");
    return thread_at(idx);
}


stack_frame_base target::selected_frame_base() {
    return thread_at(current_thread_index()).selected_frame();
}


}
