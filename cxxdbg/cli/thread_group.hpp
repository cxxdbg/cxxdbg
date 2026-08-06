// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_group.hpp
/// Contains definition of the thread_group class.

#pragma once

#include "command_group.hpp"


namespace cxxdbg::dbg::cli {


class exec_processor;
class thread_processor;


/// thread command group
class thread_group: public command_group {
public:
    /// Constructor, makes thread group with specified references to
    /// exec processor and thread processor
    thread_group(exec_processor & exec_proc, thread_processor & thread_proc);

    /// Destructor, destroys object
    virtual ~thread_group();

private:
    /// Registers exec command
    void reg_exec(exec_processor & exec_proc,
                  const std::string & name,
                  const std::string & desc);

    void reg_step_in(thread_processor & proc);
    void reg_step_over(thread_processor & proc);
    void reg_step_out(thread_processor & proc);
    void reg_select(thread_processor & proc);
    void reg_until(thread_processor & proc);
};


}


