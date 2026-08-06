// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_processor.hpp
/// Contains definition of the process_processor class.

#pragma once

#include "cxxdbg/async/result.hpp"
#include <functional>
#include <filesystem>


namespace cxxdbg::dbg::cli {


/// Abstract processor of commands from process group
class process_processor {
public:
    /// Attach completion handler
    typedef async::result_handler<unsigned long> process_handler;

    /// Destructor, destroys object
    virtual ~process_processor();

    /// Attaches to process with specified id
    virtual void attach_pid(unsigned long pid, const process_handler & handler) = 0;

    /// Attaches to process with specified name
    virtual void attach_name(const std::string & name, const process_handler & handler) = 0;

    /// Asks user to enter ID of process and attaches to id
    virtual void attach(const process_handler & handler) = 0;

    /// Continues execution of current process
    virtual void resume(const process_handler & handler) = 0;

    /// Detaches from current process
    virtual void detach(const process_handler & handler) = 0;

    /// Interrupts current process
    virtual void interrupt(const process_handler & handler) = 0;

    /// Kills current process
    virtual void kill(const process_handler & handler) = 0;

    /// Launches debugging of current target with specified launch flags
    virtual void launch(const std::filesystem::path & work_dir,
                        const std::vector<std::string> & cmd_args,
                        bool save_default,
                        const process_handler & handler) = 0;
};


}


