// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 23.06.17.
//

#pragma once

#include "cxxdbg/app/tree_view_model_adapter.hpp"
#include "source_tree.hpp"
#include "cxxdbg/app/tree_view_model.hpp"


namespace cxxdbg::dbg {


/// Node adapter for source tree view model
class source_tree_view_model_adapter {
public:
    using image_index = ro_tree_view_model::image_index;
    using color_index = ro_tree_view_model::color_index;

    static constexpr image_index image_index_file = 1;
    static constexpr image_index image_index_directory = 2;

    /// Returns number for columns in tree view model
    constexpr size_t columns_size() const { return 1; }

    /// Returns column name
    std::wstring column_name(size_t idx) const {
        assert(idx == 0 && "invalid column index");
        return L"sources";
    }

    /// Returns text for specifeid item and column
    std::wstring text(const source_tree_item * item, size_t idx) const {
        assert(idx == 0 && "invalid column index");
        return item->name();
    }

    /// Returns image for specified item and column
    image_index image(const source_tree_item * item, size_t idx) const {
        assert(idx == 0 && "invalid column index");
        if (item->is_directory()) {
            return image_index_directory;
        }

        return image_index_file;
    }

    /// Returns index of color for specified item and column
    constexpr color_index color(const source_tree_item * item, size_t idx) const {
        return 0;
    }
};


class source_tree_model: public ro_tree_view_model_adapter<source_tree, source_tree_view_model_adapter> {
public:
    static constexpr image_index image_index_file = source_tree_view_model_adapter::image_index_file;
    static constexpr image_index image_index_directory = source_tree_view_model_adapter::image_index_directory;

    /// Constructs model
    source_tree_model(const source_tree & trees):
        ro_tree_view_model_adapter<source_tree, source_tree_view_model_adapter>{trees} {}

    /// Returns index of item with specified path
    row_index path_index(const std::filesystem::path & p) const {
        assert(!p.empty() && "invalid path");
        auto item = base().path_item(p);
        return node_to_row(item);
    }
};


}
