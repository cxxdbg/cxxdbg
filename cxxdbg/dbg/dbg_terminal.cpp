// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file exe_terminal.cpp
/// Contains implementation of the exe_terminal class.

#include "dbg_terminal.hpp"
#include "application.hpp"
#include "target.hpp"
#include "cxxdbg/util/convert.hpp"


namespace cxxdbg::dbg {


void dbg_terminal::send_stdin(const std::string & data) {
    assert(can_send_stdin() && "can't send stdin");
    targ_->send_stdin(data);
}


bool dbg_terminal::can_send_stdin() const {
    if (targ_ == nullptr) {
        return false;
    }

    if (targ_->state() != target::state_t::running &&
        targ_->state() != target::state_t::stopped) {
        return false;
    }

    // can't send input if target is launched in terminal
    if (targ_->launched_in_term())
        return false;

    return true;
}


void dbg_terminal::connect(target * targ) {

    assert(targ_ == nullptr && "target already connected");

    targ_ = targ;

    can_send_stdin_changed()();

    // listening for target state changing and notifying client
    // about chaning can_send_stdin and exit after changing state of target
    targ_->connect_state_changed([this]() {
        if (targ_->launched_in_term()) {
            return;
        }

        if (targ_->state() == target::state_t::launching) {
            started()({});
        } else if (targ_->state() == target::state_t::loaded) {
            if (was_running_) {
                exited()(targ_->exit_code());
                was_running_ = false;
            }
        } else {
            // target is in one of running states
            was_running_ = true;
        }

        can_send_stdin_changed()();
    });

    // listening for target stdout
    targ_->stdout_received().connect([this](const std::string & data) {
        stdout_received()(util::convert::to_wstring(data));
    });

    // listening for target stderr
    targ_->stderr_received().connect([this](const std::string & data) {
        stderr_received()(util::convert::to_wstring(data));
    });
}


void dbg_terminal::disconnect() {
    targ_ = nullptr;
    can_send_stdin_changed()();
}


}
