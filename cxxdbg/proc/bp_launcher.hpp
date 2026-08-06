// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_launcher.hpp
/// Contains definition of the bp_launcher class.

#pragma once

#include "launcher.hpp"
#include "cxxdbg/proc/launch_parameters.hpp"
#include <boost/asio/executor.hpp>


namespace cxxdbg::proc {


/// Process launcher via boost process library
class bp_launcher: public launcher {
public:
    /// Constructs launcher with specified ASIO executor for posting events
    bp_launcher(boost::asio::executor e_exec): event_exec_{std::move(e_exec)} {}

    /// Launches child process. Returns monitor associated with launched process.
    std::unique_ptr<monitor> launch(const launch_parameters & pars) override;

private:
    boost::asio::executor event_exec_;          ///< ASIO executor for posting events
};


}
