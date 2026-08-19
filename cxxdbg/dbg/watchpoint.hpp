// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watchpoint.hpp
/// Contains definition of the watchpoint class.

#pragma once

#include "breakpoint.hpp"
#include "breakpoint_site.hpp"


namespace cxxdbg::dbg {


/// Watchpoint
class watchpoint: public breakpoint {
public:
    /// Type of watchpoint
    enum watchpoint_type { type_var_wpoint, type_expr_wpoint };

    /// Constructs watchpoint with specified number
    explicit watchpoint(breakpoint_num n, bool read = false, bool write = true, size_t size = 0);

    /// Virtual destructor
    ~watchpoint() override = default;

    /// Returns watchpoint name
    std::string name() const override = 0;

    /// Returns watchpoint type
    virtual watchpoint_type type() const = 0;

    // breakpoint_site interface implementation
    /// @return child at position
    const breakpoint_site * child_at(std::size_t position) const override;

    /// @return number of children
    std::size_t children_size() const override ;

    /// @return essential location
    const breakpoint_location * get_single_location() const override;

    // watchpoint properties
    /// Returns true if watchpoints checks for reading and false otherwise
    bool is_read() const { return read_; }

    /// Returns true if watchpoint checks for writing and false otherwise
    bool is_write() const { return write_; }

    /// Returns size
    size_t size() const { return size_; }

    /// Sets old value
    void set_old_val(const std::wstring & old_val) { old_val_ = old_val; }

    /// Sets new value
    void set_new_val(const std::wstring & new_val) { new_val_ = new_val; }

    /// Sets address
    void set_address(uint64_t address) { address_ = address; }

    /// Sets code position
    void set_position(const source_position & pos) { pos_ = pos; }

    /// Returns old value
    std::wstring old_val() const { return old_val_; }

    /// Returns new value
    std::wstring new_val() const { return new_val_; }

    /// Returns address
    uint64_t address() const { return address_; }

    /// Returns code position
    source_position get_source_position() const override;

    /// Updates watchpoint info
    bool update(const breakpoint_info * bpinf) override;

private:
    bool read_;
    bool write_;
    size_t size_;

    std::wstring old_val_;
    std::wstring new_val_;
    uint64_t address_;
    source_position pos_;
};


/// Variable watchpoint
class variable_watchpoint: public watchpoint {
public:
    /// Constructs object
    variable_watchpoint(breakpoint_num n, bool read, bool write, size_t size, std::string name);

    /// Destructs object
    ~variable_watchpoint() override = default;

    /// Returns watchpoint name
    std::string name() const override { return name_; }

    /// Returns watchpoint type
    watchpoint_type type() const override { return type_var_wpoint; }

private:
    std::string name_;
};


// Expression watchpoint
class expression_watchpoint: public watchpoint {
public:
    /// Constructs object
    expression_watchpoint(breakpoint_num n, bool read, bool write, size_t size, std::string expression);

    /// Destructs object
    ~expression_watchpoint() override = default;

    /// Returns watchpoint name
    std::string name() const override { return expression_; }

    /// Returns watchpoint type
    watchpoint_type type() const override { return type_expr_wpoint; }

private:
    std::string expression_;
};


}


