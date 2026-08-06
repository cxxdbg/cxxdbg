// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

//
// Created by yzarudny on 21.10.17.
//

#pragma once

/// \file exception_thrown_breakpoint.hpp
/// Contains definition of exception_thrown_breakpoint class

#include "code_breakpoint.hpp"


namespace cxxdbg::dbg {

class exception_thrown_breakpoint: public code_breakpoint {
public:
    /// Constructor, makes function breakpoint with specified reference to
    /// source model, id, and function name
    exception_thrown_breakpoint(source_model & smdl, num_t i);

    /// Destructor, destroys object
    virtual ~exception_thrown_breakpoint() override;

    /// Returns breakpoint name
    std::string name() const override;

    /// Returns function name used to set breakpoint
    static std::string function_name() { return "__cxa_throw"; }

private:
    /// Installs code breakpoint into implementation.
    /// Should be called for existing breakpoints after target load.
    /// Invokes handler with breakpoint info after install is finished
    void install_into_impl(breakpoint_list_impl & impl, const install_handler & h) override;
};

}

