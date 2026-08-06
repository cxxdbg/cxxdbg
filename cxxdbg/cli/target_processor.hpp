// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_processor.hpp
/// Contains definition of the target_processor class.

#pragma once

#include "forward.hpp"
#include "cxxdbg/async/forward.hpp"
#include <filesystem>
#include <functional>


namespace cxxdbg::dbg::cli {


/// Abstract processor of target commands
class target_processor {
public:
    /// Type of completion handler
    typedef async::result_handler<> completion_handler;

    /// Destructor, destroys object
    virtual ~target_processor();

    /// Loads target with specified file name and launch options
    virtual void load_target(const std::filesystem::path & file_name,
                             const std::filesystem::path & work_dir,
                             const std::vector<std::string> & cmd_args,
                             const completion_handler & handler) = 0;

    /// Closes current target
    virtual void close_target(const completion_handler & handler) = 0;
};


}


