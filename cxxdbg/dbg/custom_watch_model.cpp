// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file custom_watch_model.cpp
/// Contains implementation of the custom_watch_model class.

#include "custom_watch_model.hpp"
#include "custom_watch_list.hpp"


namespace cxxdbg::dbg {


custom_watch_model::custom_watch_model(custom_watch_list & wlist):
watch_model{wlist},
watch_{wlist} {
}


custom_watch_model::~custom_watch_model() {
}


bool custom_watch_model::editable(const row_index & r, std::size_t c) {
    // only first column of top level rows is editable
    return !parent(r) && c == 0;
}


void custom_watch_model::set_text(const row_index & r, std::size_t c, const std::wstring & s) {
    assert(c == 0 && "can't edit not first column");
    assert(!parent(r) && "can't edit not root row");

    // removing watch if text is empty
    if (s.empty()) {
        remove(index(r));
        return;
    }

    // updating watch in watch list
    const watch_list::tree_node * node = row_node(r);
    watch_.set_watch(node, s);
}


void custom_watch_model::add(const std::wstring & s) {
    watch_.add_watch(s);
}


void custom_watch_model::remove(std::size_t index) {
    watch_.remove_watch(row_node(child({}, index)));
}


}
