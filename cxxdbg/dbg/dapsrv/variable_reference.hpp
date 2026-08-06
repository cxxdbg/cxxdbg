// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file variable_reference.hpp
/// Contains definition of classes for handling references to variables

#pragma once

#include "stack_frame_compare.hpp"
#include "cxxdbg/dbg/core/stack_frame.hpp"
#include "dbgfmt/fmt_result.hpp"
#include <map>


namespace cxxdbg::dbg::core {
    class target;
}


namespace cxxdbg::dbg::dapsrv {


class variable_reference_table;


/// Base class for all variable references
class variable_reference {
public:
    /// Constructs variable reference with specified reference to var ref table and ID
    variable_reference(variable_reference_table & var_t, size_t i): var_tab_{var_t}, id_{i} {}

    /// Virtual destructor
    virtual ~variable_reference() {}

    /// Returns referene to variable reference table
    variable_reference_table & var_tab() { return var_tab_; }

    /// Returns reference ID
    size_t id() const { return id_; }

    /// Returns count of named child variables
    virtual size_t vars_size() const = 0;

    /// Returns formatted child variable with specified index
    virtual dbgfmt::named_fmt_result var_at(size_t idx) = 0;

private:
    variable_reference_table & var_tab_;            ///< Reference to var ref table
    size_t id_;                                     ///< Reference ID
};


/// Variable reference for frame scope
class frame_variable_reference: public variable_reference {
public:
    /// Constucts frame reference for specified stack frame
    frame_variable_reference(variable_reference_table & vtab, size_t id, const core::stack_frame & frm):
        variable_reference(vtab, id), frame_{frm} {}

    /// Returns count of named child variables
    size_t vars_size() const override;

    /// Returns formatted child variable with specified index
    dbgfmt::named_fmt_result var_at(size_t idx) override;

private:
    core::stack_frame frame_;   ///< Stack frame
};


/// Variable reference for stack frame locals
class locals_variable_reference: public frame_variable_reference {
public:
    /// Constructs locals variable reference to specified stack frame
    locals_variable_reference(variable_reference_table & vtab, size_t id, const core::stack_frame & frm):
        frame_variable_reference(vtab, id, frm) {}
};


/// Variable reference for stack frame parameters
class parameters_variable_reference: public frame_variable_reference {
public:
    /// Constructs paramters variable reference to specified stack frame
    parameters_variable_reference(variable_reference_table & vtab, size_t id, const core::stack_frame & frm):
        frame_variable_reference(vtab, id, frm) {}
};


/// Variable reference for stack frame registers
class registers_variable_reference: public variable_reference {
public:
    /// Constructs registers variable reference for specified stack frame
    registers_variable_reference(variable_reference_table & vtab, size_t id, const core::stack_frame & frm):
        variable_reference(vtab, id), frm_{frm} {}

    /// Returns count of named child variables
    size_t vars_size() const override;

    /// Returns formatted child variable with specified index
    dbgfmt::named_fmt_result var_at(size_t idx) override;

private:
    core::stack_frame frm_;         ///< Stack frame
};


/// Variables reference for formatting result. Used to reference child values
class fmt_result_variable_reference: public variable_reference {
public:
    /// Constructs reference object for formatting result
    fmt_result_variable_reference(variable_reference_table & vtab, size_t id, const dbgfmt::fmt_result_ref & fres):
        variable_reference(vtab, id), fres_{fres} {}

    /// Returns count of named child variables
    size_t vars_size() const override;

    /// Returns formatted child variable with specified index
    dbgfmt::named_fmt_result var_at(size_t idx) override;

private:
    dbgfmt::fmt_result_ref fres_;
};


/// Variable reference table. Stores all variable references and additional maps for
/// searching variable references
class variable_reference_table {
public:
    /// Constructs empty variable reference table
    variable_reference_table(core::target & t):
        targ_{t} {}

    /// Returns reference to target
    core::target & targ() { return targ_; }

    /// Clears table
    void clear();

    /// Returns pointer to variable reference with specified ID. Throws exception
    /// if variable reference does not exist
    variable_reference * var_ref(size_t id);

    /// Returns existing or creates new variable reference for locals in stack frame
    variable_reference * locals_ref(const core::stack_frame & frm);

    /// Returns existing or creates new variable reference to registers in stack frame
    variable_reference * registers_ref(const core::stack_frame & frm);

    /// Returns existing or creates new variable reference to formatting result
    variable_reference * fmt_result_ref(const dbgfmt::fmt_result_ref & fres);

private:
    /// Reference to target being debugged
    core::target & targ_;

    /// Map of variable references
    std::map<size_t, std::unique_ptr<variable_reference>> var_refs_;

    /// Map from frame ID to locals variable references
    std::map<core::stack_frame, variable_reference*, stack_frame_compare> local_var_refs_;

    /// Map from frame ID to registers variable reference
    std::map<core::stack_frame, variable_reference*, stack_frame_compare> registers_var_refs_;

    // /// Map from frame ID to parameters variable references
    // std::map<size_t, variable_reference*> param_var_refs_;

    /// Map from pointers to formatting result to variable references
    std::map<void*, variable_reference*> fmt_result_refs_;

    /// Next ID of variable reference
    size_t next_var_ref_id_ = 1;
};


}
