// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process_launcher.hpp
/// Contains definition of the cxxdbg::ide::process_launcher class.

#pragma once

#include "async_launcher.hpp"
#include "monitor.hpp"
#include <exception>
#include <filesystem>
#include <functional>
#include <memory>
#include <boost/signals2/signal.hpp>


namespace cxxdbg::proc {


/// Interface for abstract process launcher that provides methods for
/// launching child processes, monitoring their statuses, and reading output
class launcher: public async_launcher {
public:
    /// Virtual destructor
    virtual ~launcher() = default;

    /// Launches child process with specified command line.
    /// Returns object associated with launched process.
    virtual std::unique_ptr<monitor> launch(const launch_parameters & pars) = 0;

    /// Implementation of asynchronous launching. Calls launch and invokes handler immediately
    /// after launch functions returns.
    void launch_async(const launch_parameters & pars, const handler & handl) override {
        try {
            handl(launch(pars), {});
        }
        catch (std::exception & err) {
            handl({}, err.what());
        }
    }
};


}


