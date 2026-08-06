// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file platform.hpp
/// Cotnains definition of platform description classes.

#pragma once

#include "cxxdbg/util/range_utils.hpp"
#include <memory>
#include <string>
#include <vector>


namespace cxxdbg::dbg {


/// Describes single platform supported in debugger
class platform {
public:
    /// Constructs platform with specified name and description
    platform(std::string nm, std::string desc):
        name_{std::move(nm)}, desc_{std::move(desc)} {}

    /// Virtual destructor
    virtual ~platform() = default;

    /// Returns platform name
    auto & name() const { return name_; }

    /// Returns platform description
    auto & desc() const { return desc_; }

    /// Returns name of target specification field. Empty string means that
    /// platform uses standard launch parameters (path to exe, arguments, work dir)
    virtual std::string target_field_name() const { return {}; }

private:
    std::string name_;          ///< Platform name
    std::string desc_;          ///< Platform description
};


/// List of supported platforms
class platform_list {
public:
    /// Constructs list of all supported platforms
    platform_list();

    /// Returns range of all platforms in list
    const auto & all() const { return platforms_r_; }

    /// Returns pointer to host platform
    const platform * host() const;

private:
    /// Registers platform with specified name and description
    void register_platform(std::string name, std::string desc);

    std::vector<std::unique_ptr<platform>> platforms_;           ///< Vector of all platforms

    /// Range of const pointers to all platforms
    decltype(util::make_const_ptr_range(platforms_)) platforms_r_ = util::make_const_ptr_range(platforms_);
};


/// Platform connection options
struct platform_connection_options {
    const platform * plat = nullptr;                ///< Pointer to platform
    std::string url;                                ///< Connection URL
    std::string local_cache_dir;                    ///< Directory for storing local cache files
    bool enable_rsync = false;                      ///< Enable rsync
    std::string rsync_opts;                         ///< rsync options
    std::string rsync_prefix;                       ///< rsync prefix
    bool ignore_remote_host_name = false;           ///< Ignore host name in rsync
    std::vector<std::string> exec_search_paths;     ///< Executable/libraries search path
};


}
