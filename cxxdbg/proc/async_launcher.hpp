// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_launcher.hpp
/// Contains definition of the async_launcher class.

#pragma once

#include "monitor.hpp"
#include "launch_parameters.hpp"
#include <filesystem>


namespace cxxdbg::proc {


/// Interface for asynchonous process launcher
class async_launcher {
public:
    /// Type of launch completion handler
    using handler = std::function<void (std::unique_ptr<monitor> &&, const std::string &)>;

    /// Virtual destructor
    virtual ~async_launcher() = default;

    /// Launches child process asynchronously. Calls completion handler with child monitor after launch is complete
    virtual void launch_async(const launch_parameters & pars, const handler & handl) = 0;
};


}
