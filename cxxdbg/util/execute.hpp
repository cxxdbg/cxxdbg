// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file execute.hpp
/// Contains definition of the execute function.

#pragma once

#include <boost/asio/io_context.hpp>
#include <future>
#include <sstream>
#include <string>
#include "cxxdbg/boost_process/process.hpp"
#include <ranges.hpp>

#ifdef _WIN32
#include "cxxdbg/boost_process/windows.hpp"
#endif

#ifdef _WIN32
#define PLATFORM_BP_ARGS ,boost::process::windows::create_no_window
#else
#define PLATFORM_BP_ARGS
#endif


namespace cxxdbg::util {


namespace exec_detail {
    /// Trims string from start (in place)
    inline void ltrim(std::string & s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    }

    // Trims stromg from end (in place)
    inline void rtrim(std::string & s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    }

    // Trims string from both ends (in place)
    inline void trim(std::string & s) {
        ltrim(s);
        rtrim(s);
    }
}


/// Constructs string from command line range of arguments
template <typename ArgsRange>
std::string cmd_args_to_string(const std::string & cmd, ArgsRange && args) {
    std::ostringstream str;
    str << '"' << cmd << '"';

    if (!std::ranges::empty(args)) {
        str << " [";
        bool first = true;

        for (auto && arg : args) {
            if (first) {
                first = false;
            } else {
                str << ", ";
            }

            str << '"' << arg << '"';
        }

        str << "]";
    }

    return str.str();
}


/// Executes command with logging and checking result. Returns command output
template <typename ArgsRange>
std::string execute(const std::string & cmd, ArgsRange && args, bool throw_on_error = true) {
    std::string out, err;

    int ecode = 0;
    try {
        // launching process and reading output
        std::future<std::vector<char>> out_f, err_f;
        boost::asio::io_context io_ctx;
        boost::process::child child{cmd,
                                    boost::process::args = args,
                                    boost::process::std_out > out_f,
                                    boost::process::std_err > err_f,
                                    io_ctx
                                    PLATFORM_BP_ARGS};
        io_ctx.run();
        child.join();
        ecode = child.exit_code();

        auto out_v = out_f.get();
        out.assign(out_v.data(), out_v.size());

        auto err_v = err_f.get();
        err.assign(err_v.data(), err_v.size());
    }
    catch (std::exception & err) {
        if (throw_on_error) {
            std::ostringstream msg;
            auto args_s = cmd_args_to_string(cmd, args);
            msg << "error executing command " << args_s << ": " << err.what();
            throw std::runtime_error{msg.str()};
        }
    }

    // trimming result
    exec_detail::trim(out);

    // checking exit code
    if (ecode == 0 || !throw_on_error) {
        return out;
    }

    std::ostringstream msg;
    msg << "error executing command " << cmd_args_to_string(cmd, std::forward<ArgsRange>(args))
                                      << ": exit code: " << ecode << ", output: " << out << err;
    throw std::runtime_error{msg.str()};
}


/// Executes command with logging and checking result
inline std::string execute(const std::string & cmd, const std::initializer_list<std::string> & args, bool throw_on_error = true) {
    return execute<const std::initializer_list<std::string> &>(cmd, args, throw_on_error);
}


}
