// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_position_breakpoint.hpp
/// Contains definition of source_position_breakpoint class.

#pragma once

#include "code_breakpoint.hpp"
#include "source_position_info.hpp"


namespace cxxdbg::dbg {


/// \class source_position_breakpoint
/// Represents breakpoint at specified source position
class source_position_breakpoint: public code_breakpoint {
public:
    /// Constructor, makes breakpoint with sepcified reference to
    /// source model, ID, and source position info
    source_position_breakpoint(source_model & smdl,
                               id_type i,
                               const source_position_info & pos);

    /// Destructor, destroys object
    virtual ~source_position_breakpoint() override;

    /// Returns source position
    const source_position_info & pos() const;

    /// Sets source position
    void set_pos(const source_position_info & p);

    /// Returns breakpoint name
    std::string name() const override;

private:
    /// Installs code breakpoint into implementation.
    /// Should be called for existing breakpoints after target load.
    /// Invokes handler with breakpoint info after install is finished
    void install_into_impl(breakpoint_list_impl & impl, const install_handler & h) override;

    source_position_info pos_;          ///< Breakpoint position
};


}


