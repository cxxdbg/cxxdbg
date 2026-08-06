// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <string>
#include <list>
#include <iosfwd>


namespace cxxdbg::dbg {


/// Key type for expand state tree node
class tree_node_key {
public:
    /// Default constructor
    tree_node_key() = default;

    /// Constructor
    tree_node_key(const std::wstring & name, const std::wstring & type);

    /// Copy constructor
    tree_node_key(const tree_node_key & key);

    /// operator=
    tree_node_key & operator=(const tree_node_key & key);

    bool operator==(const tree_node_key & other) const {
        return name_ == other.name() && type_ == other.type();
    }

    /// Returns name
    const std::wstring & name() const  { return name_; }

    /// Returns type
    const std::wstring & type() const { return type_; }

    /// Sets name
    void set_name(const std::wstring & name) { name_ = name; }

    /// Sets type
    void set_type(const std::wstring & type) { type_ = type; }

    /// Returns true if key is empty (name and type both are empty)
    bool is_empty() const { return name_.empty() && type_.empty(); }

    /// for debug purposes
    friend std::wostream & operator << (std::wostream & s, tree_node_key & k);

private:
    std::wstring name_; ///< Name
    std::wstring type_; ///< Type
};

/// represents path in a tree counting from a root node
typedef std::list<tree_node_key> tree_node_path;

std::wostream & operator<<(std::wostream & s, tree_node_path & path);


}


