// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file template.hpp
/// Contains declarations of utility functions for parsing template names

#pragma once

#include <istream>
#include <sstream>
#include <string>
#include <vector>


namespace cxxdbg { namespace util {


/// Rperesents template instantiation. Contains info about
/// template name and parameters
class template_instantiation {
public:
    /// Constructor, makes template instantiation with specified template name
    template_instantiation(const std::string & nm);

    /// Returns reference to vector of template parameters
    std::vector<template_instantiation> pars;

private:
    std::string name_;                  ///< Template name
    std::vector<std::string> pars_;     ///< Template parameters
};


/// Parses template name from input stream
void parse_template_name(std::istream & str,
                         std::string & name,
                         std::vector<std::string> & pars);


} }


