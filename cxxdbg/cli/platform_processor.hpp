// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file platform_processor.hpp
/// Contains definition of the platform_processor class.

#pragma once

#include "cxxdbg/async/result.hpp"
#include <string>


namespace cxxdbg::dbg::cli {


/// Processor of the platform commands group
class platform_processor {
public:
    /// Selects platform
    virtual bool select_platform(std::string_view name) = 0;

    /// Connects to platform. Calls completion handler with selected platform name
    virtual void connect_to_platform(const std::string_view url,
                                     const std::string & local_cache_dir,
                                     bool enable_rsync,
                                     const std::string & rsync_opts,
                                     const std::string & rsync_prefix,
                                     bool ignore_remote_host_name,
                                     const async::result_handler<std::string> & handler) = 0;

    /// Disconnects from platform
    virtual void disconnect_from_platform() = 0;
};


}
