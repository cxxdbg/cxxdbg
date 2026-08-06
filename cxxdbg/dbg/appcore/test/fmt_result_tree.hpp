// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "../watch_list_server.hpp"
#include "dbgfmt/fixed_fmt_result.hpp"
#include "dbgfmt/fmt_result.hpp"

#include <string>
#include <deque>


namespace cxxdbg::dbg::test {


/// class fmt_result node is used for constructing trees of dbgfmt::named_fmt_result for testing purposes
class fmt_result_node {
public:
    /// Constructor
    fmt_result_node(std::wstring name, std::wstring type, std::vector<fmt_result_node> childs = {}):
        name_(std::move(name)), type_(std::move(type)), childs_(std::move(childs)) {
    }

    fmt_result_node(const fmt_result_node &) = default;
    fmt_result_node(fmt_result_node &&) = default;

    /// Destructor, destroys object
    ~fmt_result_node() = default;

    /// Returns node name
    const std::wstring & name() const { return name_; }

    /// Returns value of the value field
    std::wstring val() const { return {}; }

    /// Returns value of type field
    std::wstring type() const { return type_; }

    /// Returns source position of definition of object that is contained in address
    /// specied by value
    dbgfmt::backend::source_position val_pos() const { return {}; }

    /// Returns source position of definition of value type
    dbgfmt::backend::source_position type_pos() const { return {}; }

    /// Constructs formatting result from this node
    dbgfmt::fmt_result_ref make_fmt_result() const {
        auto res = std::make_unique<dbgfmt::fixed_fmt_result>(std::wstring{},
                                                           type_,
                                                           dbgfmt::backend::source_position{},
                                                           dbgfmt::backend::source_position{});
        for (auto && ch : childs_) {
            res->add_child(ch.name(), [ch]() {
                return ch.make_fmt_result();
            });
        }

        return dbgfmt::make_ref(std::move(res));
    }

    /// Constructs named formatting result from this node
    dbgfmt::named_fmt_result make_named_fmt_result() const {
        return dbgfmt::named_fmt_result{name(), {}, make_fmt_result()};
    }

    operator dbgfmt::named_fmt_result() const {
        return make_named_fmt_result();
    }

private:
    std::wstring name_;                             ///< node name
    std::wstring type_;                             ///< node type
    std::vector<fmt_result_node> childs_;           ///< children
};


/// class fmt_result_node_list keeps list of nodes
class fmt_result_node_list {
public:
    fmt_result_node_list() {
    };

    using watch_id = watch_list_server::watch_id;

    fmt_result_node_list & add(watch_id wid, fmt_result_node && res) {
        auto tmp = std::make_shared<fmt_result_node>(res);
        roots_.push_back({wid, tmp});
        return * this;
    }

    auto make_variables() {
        std::map<watch_id , dbgfmt::named_fmt_result> vars;
        for (auto & it : roots_) {
            vars.insert({it.first, it.second->make_named_fmt_result()});
        }

        return std::move(vars);
    }

private:
    std::vector< std::pair<watch_id, std::shared_ptr<fmt_result_node>> > roots_;
};


}



