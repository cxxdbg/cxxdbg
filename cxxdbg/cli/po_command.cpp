// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file po_command.cpp
/// Contains implementation of the po_command class and related classes.

#include "po_command.hpp"


namespace cxxdbg::dbg::cli {


po_command_base::po_command_base(
        const std::string & desc,
        const std::string & hmsg,
        const boost::program_options::options_description & odesc,
        const boost::program_options::positional_options_description & podesc):
command{desc},
help_msg_{hmsg},
opt_desc_{odesc},
popt_desc_{podesc} {
}


po_command_base::~po_command_base() {
}


void po_command_base::help(const string_vector & cmd,
                                                const completion_handler & handler) const {
    std::ostringstream msg;

    // displaying help message
    msg << help_msg_ << "\n\n";

    // displaying options description
    msg << "Command options:\n";
    msg << opt_desc_ << "\n";

    handler(msg.str());
}


void po_command_base::set_help_msg(const std::string & hmsg) {
    help_msg_ = hmsg;
}


void po_command_base::set_desc_help(const std::string & s) {
    set_desc(s);
    set_help_msg(s);
}


const boost::program_options::options_description &
po_command_base::opt_desc() const {
    return opt_desc_;
}


boost::program_options::options_description &
po_command_base::opt_desc() {
    return opt_desc_;
}


const boost::program_options::positional_options_description &
po_command_base::popt_desc() const {
    return popt_desc_;
}


boost::program_options::positional_options_description &
po_command_base::popt_desc() {
    return popt_desc_;
}



}
