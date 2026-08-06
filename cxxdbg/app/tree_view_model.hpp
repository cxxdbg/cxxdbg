// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file tree_view_model.hpp
/// Contains definition of the tree_view_model class.

#pragma once

#include "signals.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>


namespace cxxdbg {


/// Expand state enumeration
enum class tree_view_expand_state {
    undefined, collapsed, expanded
};


/// Row index in tree view model
class tree_view_model_row_index {
    friend class ro_tree_view_model;

public:
    /// Default constructor, makes invalid row index
    tree_view_model_row_index();

    /// Destructor, destroys object
    ~tree_view_model_row_index();

    /// Returns internal pointer
    void * ptr() const;

    /// Return internal uintptr_t value
    uintptr_t val() const;

    /// Returns true if index is valid
    bool is_valid() const;

    /// Returns true if index is valid
    operator bool() const;

    /// Returns true if index is invalid
    bool operator !() const;

    /// Compares two indexes
    bool operator==(const tree_view_model_row_index & r) const;

    /// Compares two indexes
    bool operator!=(const tree_view_model_row_index & r) const;

private:
    /// Constructor, makes row index with specified internal poiner
    explicit tree_view_model_row_index(void * p);

    /// Constructor, makes row index with specified uintptr_t value
    explicit tree_view_model_row_index(uintptr_t v);

    void * ptr_;        ///< Internal pointer
};


/// Represents abstract read only model for tree view widgets
class ro_tree_view_model {
public:
    typedef tree_view_model_row_index row_index;
    typedef void childs_signal_func(const row_index&, size_t, size_t);
    typedef void row_index_signal_func(const row_index &);
    typedef void rows_range_signal_func(const row_index &, const row_index &);
    typedef void childs_move_func(const row_index&, size_t, size_t,
                                  const row_index&, size_t);
    typedef signal<void (const row_index &, size_t, size_t)> childs_signal;
    typedef signal<void (const row_index &)> row_index_signal;
    typedef size_t image_index;
    typedef size_t color_index;

    /// Destructor, destoys object
    virtual ~ro_tree_view_model() {}

    /// Returns number of columns
    virtual std::size_t columns_size() const = 0;

    /// Returns name of column with specified index
    virtual std::wstring column_name(std::size_t index) const = 0;

    /// Returns number of child rows for row
    virtual std::size_t childs_size(const row_index & row) const = 0;

    /// Returns child row with specified index
    virtual row_index child(const row_index & row, std::size_t index) const = 0;

    /// Returns index of row in parent
    virtual std::size_t index(const row_index & row) const = 0;

    /// Returns parent for specified row
    virtual row_index parent(const row_index & row) const = 0;

    /// Returns text for specified cell
    virtual std::wstring text(const row_index & row, std::size_t c) const = 0;

    /// Returns image index for specified cell. Default implementation returns 0.
    virtual image_index image(const row_index & row, std::size_t c) const;

    /// Returns color index for specified cell. Default implementation returns 0.
    virtual color_index color(const row_index & row, std::size_t c) const;

    /// Returns true if model has expand_state interface support and false otherwise, no support by default
    virtual bool has_expand_state_support() const { return false; }

    /// Returns expand state of selected index, undefined by default
    virtual tree_view_expand_state get_expand_state(const row_index & index) const {
        return tree_view_expand_state::undefined;
    }

    /// Sets expand state
    virtual void set_expand_state(const row_index & index, tree_view_expand_state state) const {};

    /// Before added signal, emitted before adding new row
    CXXDBG_DEFINE_SIGNALX(before_added, childs_signal_func)

    /// After added signal, emitted after adding new row
    CXXDBG_DEFINE_SIGNALX(after_added, childs_signal_func)

    /// Before removed signal, emitted before removing row
    CXXDBG_DEFINE_SIGNALX(before_removed, childs_signal_func)

    /// After removed signal, emitted after removig row
    CXXDBG_DEFINE_SIGNALX(after_removed, childs_signal_func)

    /// Text changed signal, emitted after text in cell in specified
    /// row is changed
    CXXDBG_DEFINE_SIGNALX(after_changed, row_index_signal_func)

    /// Multiple rows data changed signal, emitted after model update
    CXXDBG_DEFINE_SIGNALX(after_changed_multy, rows_range_signal_func)

    /// Called before layout of child nodes changed
    CXXDBG_DEFINE_SIGNALX(before_layout_changed, row_index_signal_func)

    /// Called after layout of child nodes has been changed
    CXXDBG_DEFINE_SIGNALX(after_layout_changed, row_index_signal_func)

    /// Called when node needs to be visually expanded
    CXXDBG_DEFINE_SIGNALX(expand_row, row_index_signal_func)

    /// Makes row index with specified internal pointer
    static row_index make_index(void * ptr);

    /// Makes row index with specified uintptr_t value
    static row_index make_index(uintptr_t val);
};


/// Represents abstract sortable read only model
class sortable_ro_tree_view_model: virtual public ro_tree_view_model {
public:
    /// Sort order
    enum class sort_order {
        no,
        ascending,
        descending
    };

    /// Sets sort order for model. Should be called only if sortable returns true.
    /// Default implementation does nothing and calls assert(false)
    virtual void sort(size_t column, sort_order order) {
        assert(false && "sort should not be called for tree_view_model");
    }
};


/// Represents abstract editable model for tree view widgets
class tree_view_model: virtual public ro_tree_view_model {
public:
    /// Destructor, destroys object
    ~tree_view_model() override = default;

    /// Returns true if cell is editable
    virtual bool editable(const row_index & r, std::size_t c) = 0;

    /// Sets text for specified cell
    virtual void set_text(const row_index & r, std::size_t c, const std::wstring & s) = 0;
};


/// Represents abstract sortable and editable model for tree view widgets
class sortable_tree_view_model: virtual public tree_view_model,
                                virtual public sortable_ro_tree_view_model {
public:
    ~sortable_tree_view_model() override = default;
};


/// Converts tree view expand state to string
std::string tree_view_expand_state_to_string(tree_view_expand_state st);


}


namespace std {
    /// Hash function specialization for tree view model index.
    /// Returns hash of internal pointer
    template <> struct hash<cxxdbg::tree_view_model_row_index> {
        using argument_type = cxxdbg::tree_view_model_row_index;
        using result_type = size_t;

        result_type operator()(const argument_type & arg) const {
            return hash<void*>()(arg.ptr());
        }
    };
}


