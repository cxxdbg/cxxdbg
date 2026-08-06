// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "treewalk.hpp"
#include "cxxdbg/app/tree_view_model.hpp"


namespace cxxdbg::dbg::util::tree {

auto make_model_printer(ro_tree_view_model & model) {
    using node_type = ro_tree_view_model::row_index;
    auto printer = make<node_type>::tree_walker1(
            [&model](const node_type & node, size_t shift) {   // visit
                size_t cols = model.columns_size();
                std::wcout << std::wstring(shift, L' ');
                for (size_t i = 0; i < cols; ++i)
                    std::wcout << model.text(node, i) << L' ';

                std::wcout << L" |"<< std::endl;
            },
            [&model](const node_type & node) -> size_t { return model.childs_size(node); }, // count
            [&model](const node_type & node, size_t idx) -> const node_type { return model.child(node, idx); } // at
    );

    return printer;
}

}

