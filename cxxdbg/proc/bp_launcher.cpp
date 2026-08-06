// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file bp_launcher.cpp
/// Contains implementation of the bp_launcher class.

#include "bp_launcher.hpp"
#include "bp_child_monitor.hpp"
#include "bp_utils.hpp"
#include "cxxdbg/boost_process/process.hpp"


namespace cxxdbg::proc {


std::unique_ptr<monitor> bp_launcher::launch(const launch_parameters & pars) {
    CXXDBG_PROC_LOG_INFO << "launch: " << convert_launch_pars_to_string(pars);

    // launching process
    auto bp_child = create_bp_child(pars);

    // creating process monitor
    return std::make_unique<bp_child_monitor>(event_exec_, std::move(bp_child));
}


}
