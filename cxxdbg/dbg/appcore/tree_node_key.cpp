// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "tree_node_key.hpp"
#include <iostream>


namespace cxxdbg::dbg {


std::wostream & operator<<(std::wostream & s, tree_node_key & k) {
    return s << L" {name=" << k.name_ << L", type=" << k.type_ << L"}";
}

tree_node_key & tree_node_key::operator=(const tree_node_key & key) {
    name_ = key.name();
    type_ = key.type();

    return *this;
}

tree_node_key::tree_node_key(const std::wstring & name, const std::wstring & type) :
        name_(name), type_(type) {
}

tree_node_key::tree_node_key(const tree_node_key & key) :
        name_(key.name()), type_(key.type()) {
}

std::wostream & operator<<(std::wostream & s, tree_node_path & path) {
    for (auto & p : path) {
        s << p;
    }
    return s;
}


}
