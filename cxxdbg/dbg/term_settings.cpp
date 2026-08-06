// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file term_settings.cpp
/// Contains implementation of the term_settings class.

#include "term_settings.hpp"


namespace cxxdbg::dbg {


term_settings::term_settings(term_kind knd,
                             const std::filesystem::path & xterm_p,
                             bool cls_xterm,
                             const std::string & custom_t_cmd):
tkind_(knd),
xterm_path_(xterm_p),
close_xterm_(cls_xterm),
custom_term_cmd_(custom_t_cmd) {
}


const std::filesystem::path & term_settings::xterm_path() const {
    return xterm_path_;
}


bool term_settings::close_xterm() const {
    return close_xterm_;
}


const std::string & term_settings::custom_term_cmd() const {
    return custom_term_cmd_;
}


void term_settings::set_xterm_path(const std::filesystem::path & p) {
    xterm_path_ = p;
}


void term_settings::set_close_xerm(bool v) {
    close_xterm_ = v;
}


void term_settings::set_custom_term_cmd(const std::string & cmd) {
    custom_term_cmd_ = cmd;
}


bool term_settings::operator==(const term_settings & sett) const {
    return tkind() == sett.tkind() &&
           xterm_path() == sett.xterm_path() &&
           close_xterm() == sett.close_xterm() &&
           custom_term_cmd() == sett.custom_term_cmd();
}


bool term_settings::operator!=(const term_settings & sett) const {
    return !(*this == sett);
}


}
