// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_ro_tree_view_model_test.cpp
/// Contains unit tests for the vector_ro_tree_view_model class.

#if 0

#include "../vector_ro_tree_view_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


namespace {
    struct my_model_item {
        std::wstring text;
        ro_tree_view_model::image_index image;
        ro_tree_view_model::color_index color;

        std::vector<my_model_item> child_items;
    };

    struct my_model_item_converter {
        static size_t columns_size() {
            return 0;
        }

        std::wstring column_name(size_t idx) {
            return {};
        }

        std::wstring text(const my_model_item & item, size_t c) {
            return item.text;
        }

        ro_tree_view_model::image_index image(const my_model_item & item, size_t c) {
            return item.image;
        }

        ro_tree_view_model::color_index color(const my_model_item & item, size_t c) {
            return item.color;
        }
    };
}


struct vector_ro_tree_view_model_test_fixture {
    vector_model<my_model_item> model;
    vector_ro_tree_view_model<my_model_item, my_model_item_converter> tree_model;
};


}

#endif

