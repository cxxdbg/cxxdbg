// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file term_settings.hpp
/// Contains definition of the term_settings class.

#pragma once

// The iosfwd include is workaround for the QTBUG-73263 bug that causes moc to fail to
// precompile headers that include <filesystem> header. The <ioswfd> include fixes the problem.
// See https://bugreports.qt.io/browse/QTBUG-73263 for more details.
#include <iosfwd>

#include <filesystem>
#include <string>


namespace cxxdbg::dbg {


/// \class term_settings
/// Represents terminal settings
class term_settings {
public:
    /// Terminal kind
    enum class term_kind {
        xterm,
        builtin,
        custom
    };

    /// Constructor, makes new terminal settings
    term_settings(term_kind knd = term_kind::builtin,
                  const std::filesystem::path & xterm_p = std::filesystem::path(),
                  bool cls_xterm = false,
                  const std::string & custom_t_cmd = std::string());

    auto tkind() const { return tkind_; }
    const std::filesystem::path & xterm_path() const;
    bool close_xterm() const;
    const std::string & custom_term_cmd() const;

    void set_tkind(term_kind knd) { tkind_ = knd; }
    void set_xterm_path(const std::filesystem::path & p);
    void set_close_xerm(bool v);
    void set_custom_term_cmd(const std::string & cmd);

    bool operator==(const term_settings & sett) const;
    bool operator!=(const term_settings & sett) const;

private:
    term_kind tkind_;                       ///< Terminal kind to use
    std::filesystem::path xterm_path_;    ///< Path to xterm
    bool close_xterm_;                      ///< True if xterm should be closed after process exit
    std::string custom_term_cmd_;           ///< Custom terminal command
};


}


