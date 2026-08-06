// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file command_line.hpp
/// Contains definition of command_line class.

#pragma once

#include <string>
#include <vector>
#include <boost/program_options.hpp>


namespace cxxdbg::dbg {


/// \class command_line
/// Represents cxxdbg command line parsing result
class command_line {
public:
    /// Constructor, makes command line with default option values
    command_line();

    /// Parses command line options
    void parse(int argc, char * argv[]);

    /// Returns true if help option was specified
    bool show_help() const { return help_; }

    /// Returns true if version option was specified
    bool show_version() const { return version_; }

    /// Prints help message to output stream
    void print_help(std::ostream & str, std::string_view product_name) const;

    /// Prints version info to output stream
    void print_version(std::ostream & str, std::string_view product_name) const;

    /// Returns executable name
    const std::string & exe_name() const { return exe_name_; }

    /// Returns platform name
    const std::string & platform_name() const { return platform_name_; }

    /// Returns platform URL
    const std::string & platform_url() const { return platform_url_; }

    /// Returns vector of executable command line arguments
    const std::vector<std::string> & exe_args() const { return exe_args_; }

    /// Returns map of variables passed to command line
    const boost::program_options::variables_map & vars() const { return vars_; }

    /// Returns vector of initialization commands
    const std::vector<std::string> & init_commands() const { return init_cmds_; }

    /// Returns vector of executable search paths
    auto & exec_search_paths() const { return exec_search_paths_; }

private:
    /// Command line options description
    boost::program_options::options_description opt_desc_;

    /// Position command line options description
    boost::program_options::positional_options_description pos_opt_desc_;

    /// Map of variables pssed to command line
    boost::program_options::variables_map vars_;

    std::string exe_name_;                      ///< Executable name
    std::string platform_name_;                 ///< Platform name
    std::string platform_url_;                  ///< Platform URL
    std::vector<std::string> exe_args_;         ///< Executable command arguments
    std::vector<std::string> init_cmds_;        ///< Initialization commands
    std::vector<std::string> exec_search_paths_;///< Executable search paths
    bool help_;                                 ///< Show help message
    bool version_;                              ///< Show version info
};


}


