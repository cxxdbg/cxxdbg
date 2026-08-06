// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file frame_group.hpp
/// Contains definition of the frame_group class.

#pragma once

#include "command_group.hpp"


namespace cxxdbg::dbg::cli {


class exec_processor;
class frame_processor;


/// \class frame_group
/// frame commands group
class frame_group: public command_group {
public:
    /// Constructor, makes frame command group with specified references
    /// to exec and frame processors
    frame_group(exec_processor & exec_proc, frame_processor & frame_proc);

    /// Destructor, destroys object
    virtual ~frame_group();

private:
    void reg_exec_cmd(exec_processor & exec_proc,
                      const std::string & name,
                      const std::string & desc);
};


}


