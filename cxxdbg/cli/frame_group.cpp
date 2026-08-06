// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file frame_group.cpp
/// Contains implementation of the frame_group class.

#include "exec_command.hpp"
#include "frame_group.hpp"
#include "frame_processor.hpp"
#include "po_command.hpp"
#include "cxxdbg/async/result.hpp"


namespace po = boost::program_options;


namespace cxxdbg::dbg::cli {


frame_group::frame_group(exec_processor & exec_proc, frame_processor & frame_proc):
command_group{"A set of commands for operating on frames"} {

    // registering exec commands
    reg_exec_cmd(exec_proc, "info", "Show information about selected frame");
    reg_exec_cmd(exec_proc, "variable", "Show frame variables");

    // adding select command
    auto select_cmd = make_po_command([&frame_proc](const po::variables_map & vars, const auto & h) {
        unsigned int index = vars["frame-index"].as<unsigned int>();
        frame_proc.select_frame(index, [h, index](const auto & r) {
            if (r.is_ok()) {
                std::ostringstream msg;
                msg << "selected frame " << index;
                h(msg.str());
            } else {
                h(r.error());
            }
        });
    });

    select_cmd->set_desc_help("Select frame");
    select_cmd->opt_desc().add_options()
            ("frame-index", po::value<unsigned int>()->required(), "Frame index");
    select_cmd->popt_desc().add("frame-index", 1);
    reg_cmd("select", select_cmd);
}


frame_group::~frame_group() {
}


void frame_group::reg_exec_cmd(exec_processor & exec_proc,
                               const std::string & name,
                               const std::string & desc) {

    reg_cmd(name, exec_command::make(exec_proc, std::string("frame ") + name, desc));
}



}

