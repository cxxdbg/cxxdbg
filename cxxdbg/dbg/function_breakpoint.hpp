// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_breakpoint.hpp
/// Contains definition of function_breakpoint class

#pragma once

#include "code_breakpoint.hpp"


namespace cxxdbg::dbg {


/// \class function_breakpoint
/// Represents single breakpoint at specified function in target being debugged
class function_breakpoint: public code_breakpoint {
public:
    /// Constructor, makes function breakpoint with specified reference to
    /// source model, id, and function name
    function_breakpoint(source_model & smdl, id_type i, const std::string & nm);

    /// Destructor, destroys object
    virtual ~function_breakpoint() override;

    /// Returns function name
    const std::string & func_name() const;

    /// Sets function name
    void set_func_name(const std::string & nm);

    /// Returns breakpoint name
    std::string name() const override;

private:
    /// Installs code breakpoint into implementation.
    /// Should be called for existing breakpoints after target load.
    /// Invokes handler with breakpoint info after install is finished
    void install_into_impl(breakpoint_list_impl & impl, const install_handler & h) override;

    std::string func_name_;         ///< Function name
};


}


