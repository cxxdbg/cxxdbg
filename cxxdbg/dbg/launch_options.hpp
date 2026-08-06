// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file launch_options.hpp
/// Contains definition of the launch_options class.

#pragma once

// The iosfwd include is workaround for the QTBUG-73263 bug that causes moc to fail to
// precompile headers that include <filesystem> header. The <ioswfd> include fixes the problem.
// See https://bugreports.qt.io/browse/QTBUG-73263 for more details.
#include <iosfwd>

#include <filesystem>
#include <string>
#include <vector>


namespace cxxdbg::dbg {


/// \class launch_options
/// Contains launch options values
class launch_options {
public:
    /// Result of options checking
    enum check_result {
        check_result_ok,
        check_result_work_dir_does_not_exist,
        check_result_work_dir_is_not_a_dir
    };

    /// Returns working directory
    const std::filesystem::path & work_dir() const { return work_dir_; }

    /// Sets working directory
    void set_work_dir(const std::filesystem::path & wd) { work_dir_ = wd; }

    /// Returns const reference to vector of launch arguments
    const std::vector<std::string> & launch_args() const { return launch_args_; }

    /// Returns reference to vector of launch arguments
    std::vector<std::string> & launch_args() { return launch_args_; }

    /// Sets launch arguments from string
    void set_launch_args(const std::string & args);

    /// Sets launch arguments
    void set_launch_args(const std::filesystem::path & exe, const std::vector<std::wstring> & args);

    /// Checks options. Returns check result
    check_result check() const;

    /// Makes launch args string from vector of args
    std::string launch_args_string() const;

    /// Compares launch options
    bool operator==(const launch_options & rhs) const;

private:
    std::filesystem::path work_dir_;      ///< Working directory
    std::vector<std::string> launch_args_;  ///< Launch arguments
};


}


