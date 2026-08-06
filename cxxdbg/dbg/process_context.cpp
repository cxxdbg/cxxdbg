// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_context.cpp
/// Contains implementation of process_context class.

#include "process_context.hpp"


namespace cxxdbg::dbg {


process_context::process_context():
pid_{0},
stop_rsn_(stop_reason_unknown) {
}


unsigned long process_context::pid() const {
    return pid_;
}


void process_context::set_pid(unsigned long p) {
    pid_ = p;
}


thread_list_info & process_context::threads() {
    return threads_;
}


const thread_list_info & process_context::threads() const {
    return threads_;
}


process_context::breakpoint_info_vector & process_context::breakpoints() {
    return breakpoints_;
}


const process_context::breakpoint_info_vector & process_context::breakpoints() const {
    return breakpoints_;
}


watch_list_impl::tree_info & process_context::locals() {
    return locals_;
}


const watch_list_impl::tree_info & process_context::locals() const {
    return locals_;
}


void process_context::set_locals(const watch_list_impl::tree_info & ltree) {
    locals_ = ltree;
}


const watch_list_impl::tree_info & process_context::watch() const {
    return watch_;
}


void process_context::set_watch(const watch_list_impl::tree_info & wtree) {
    watch_ = wtree;
}


process_context::stop_reason_t process_context::stop_reason() const {
    return stop_rsn_;
}


void process_context::set_stop_reason(stop_reason_t rsn) {
    stop_rsn_ = rsn;
}


int process_context::stop_signal() const {
    return stop_signal_;
}


void process_context::set_stop_signal(int s) {
    stop_signal_ = s;
}



process_context::module_info_vector_ptr process_context::modules() const {
    return modules_;
}


void process_context::set_modules(const module_info_vector_ptr & mods) {
    modules_ = mods;
}


}
