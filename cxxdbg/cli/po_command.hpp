// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file po_command.hpp
/// Contains definition of the po_command class and related classes.

#pragma once

#include "command.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <sstream>


namespace cxxdbg::dbg::cli {


/// Base class of all program options commands
class po_command_base: public command {
public:
    /// Constructor, makes command handler with speicifed
    /// description string
    po_command_base(
            const std::string & desc = std::string(),
            const std::string & hmsg = std::string(),
            const boost::program_options::options_description & odesc
                = boost::program_options::options_description(),
            const boost::program_options::positional_options_description & podesc
                = boost::program_options::positional_options_description());

    /// Destructor, destroys object
    virtual ~po_command_base();


    /// Returns help messge for command specified as vector of arguments
    virtual void help(const string_vector & cmd, const completion_handler & handler) const;


    /// Sets help message
    void set_help_msg(const std::string & hmsg);

    /// Sets help message and description to same string
    void set_desc_help(const std::string & s);

    /// Returns const reference to options description
    const boost::program_options::options_description & opt_desc() const;

    /// Returns reference to options description
    boost::program_options::options_description & opt_desc();

    /// Returns const reference to positional options description
    const boost::program_options::positional_options_description & popt_desc() const;

    /// Returns reference to position options description
    boost::program_options::positional_options_description & popt_desc();

private:
    std::string help_msg_;              ///< Help message

    /// Options description
    boost::program_options::options_description opt_desc_;

    /// Positional options description
    boost::program_options::positional_options_description popt_desc_;
};


/// \class po_command
/// Generic command handler which parses options using boost program options.
template <typename Func>
class po_command: public po_command_base {
public:
    /// Constructor, makes command handler with specified handler function
    po_command(const Func & func);

    /// Executes command represented as vector of arguments and calls
    /// completion handler on completion
    virtual void exec(const string_vector & cmd, const completion_handler & chandler) const;

private:
    Func handle_func_;                  ///< Handle function
};


/// Makes program options command handler
template <typename H>
std::shared_ptr<po_command<H>>
make_po_command(const H & handler) {
    return std::shared_ptr<po_command<H>> {
           new po_command<H>{handler}
    };
}


template <typename H>
po_command<H>::po_command(const H & func):
handle_func_{func} {
}


template <typename H>
void po_command<H>::exec(const string_vector & cmd,
                                              const completion_handler & chandler) const {
    // parsing options
    boost::program_options::command_line_parser parser{cmd};
    parser.options(opt_desc());
    parser.positional(popt_desc());
    boost::program_options::variables_map var_map;
    boost::program_options::store(parser.run(), var_map);
    boost::program_options::notify(var_map);

    // processing options
    handle_func_(var_map, chandler);
}


}


