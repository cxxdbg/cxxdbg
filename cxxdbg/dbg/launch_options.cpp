// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file launch_options.cpp
/// Contains implementation of the launch_options class.

#include "launch_options.hpp"
#include "cxxdbg/util/convert.hpp"
#include <filesystem>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


void launch_options::set_launch_args(const std::string & args) {

    std::string targs = boost::algorithm::trim_copy(args);
    if (targs.empty())
        return;

    boost::algorithm::split(launch_args_, targs, boost::is_any_of("\t "), boost::token_compress_on);
}


void launch_options::set_launch_args(const std::filesystem::path & exe, const std::vector<std::wstring> & args) {
    launch_args_.clear();
    launch_args_.reserve(args.size() + 1);
    launch_args_.push_back(exe.string());
    for (auto && a : args) {
        launch_args_.push_back(util::convert::to_string(a));
    }
}


launch_options::check_result launch_options::check() const {

    // empty working directory is allowed
    if (work_dir_.empty()) {
        return check_result_ok;
    }

    // checking that work dir exists
    if (!fs::exists(work_dir_))
        return check_result_work_dir_does_not_exist;

    // checking that work dir is a directory
    if (!fs::is_directory(work_dir_))
        return check_result_work_dir_is_not_a_dir;

    return check_result_ok;
}


std::string launch_options::launch_args_string() const {
    std::ostringstream str;
    bool first = true;
    for (auto it = launch_args_.begin(), end = launch_args_.end(); it != end; ++it) {
        if (!first)
            str << " ";
        else
            first = false;

        str << *it;
    }

    return str.str();
}


bool launch_options::operator==(const launch_options & rhs) const {
    return work_dir_ == rhs.work_dir_ && launch_args_ == rhs.launch_args_;
}


}
