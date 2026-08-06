// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file address_breakpoint.hpp
/// Contains definition of the addres_breakpoint class.

#pragma once

#include "code_breakpoint.hpp"


namespace cxxdbg::dbg {


/// \class address_breakpoint
/// Represents single breakpoint at specified address in target being debugged
class address_breakpoint: public code_breakpoint {
public:
    /// Constructor, makes function breakpoint with specified reference to
    /// source model, id,  and function name
    address_breakpoint(source_model & smdl, id_type i, std::uint64_t addr);

    /// Destructor, destroys object
    virtual ~address_breakpoint() override;

    /// Returns breakpoint address
    std::uint64_t address() const;

    /// Sets breakpoint address
    void set_address(uint64_t addr);

    /// Returns breakpoint name
    std::string name() const override;

private:
    /// Installs code breakpoint into implementation.
    /// Should be called for existing breakpoints after target load.
    /// Invokes handler with breakpoint info after install is finished
    void install_into_impl(breakpoint_list_impl & impl, const install_handler & h) override;

    std::uint64_t addr_;            ///< Breakpoint address
};



}


