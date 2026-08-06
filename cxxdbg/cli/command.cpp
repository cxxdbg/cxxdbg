// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command.cpp
/// Contains implementation of the command class.

#include "command.hpp"


namespace cxxdbg::dbg::cli {


command::command(const std::string & dsc):
desc_{dsc} {
}


command::~command() {
}


const std::string & command::desc(const string_vector & cmd) const {
    return desc_;
}


void command::set_desc(const std::string & d) {
    desc_ = d;
}


}
