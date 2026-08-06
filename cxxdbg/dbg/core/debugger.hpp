// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debugger.hpp
/// Contains defintion of the debugger class

#pragma once

#include "tn_code_model.hpp"
#include "debugger_base.hpp"


namespace cxxdbg::dbg::core {


class target;


/// Main CXXDBG debugger class.
class debugger: public debugger_base {
public:
    /// Constructs debugger with specified reference to event execution queue
    debugger(async::event_queue & eq);

    /// Returns reference to code model implementation
    virtual const code_model & cm() const override {
        return cm_;
    }
    
    /// Loads target with specified exe path, returns shared pointer to target
    std::shared_ptr<target> load_target(const std::filesystem::path & exe_path);

    /// Attaches to target with specified PID
    std::shared_ptr<target> attach(unsigned long pid);

    /// Attaches to target with specified process name
    std::shared_ptr<target> attach(const std::string & name);

    /// Returns reference to dummy target
    target & dummy_target();

private:
    static bool initialized_;               ///< Was debugger initialized?
    const tn_code_model cm_;                ///< Code model implementation
    std::shared_ptr<target> dummy_targ_;    ///< Pointer to dummy target
};


}
