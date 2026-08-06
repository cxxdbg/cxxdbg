// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file utils.cpp
/// Contains implementation of small utility functions.

#include "utils.hpp"
#include "code_model.hpp"
#include <lldb/API/SBDeclaration.h>
#include <lldb/API/SBFileSpec.h>
#include <lldb/Utility/FileSpec.h>


namespace fs = std::filesystem;


namespace cxxdbg::dbg::core {


fs::path lldb_sb_file_spec_to_path(const lldb::SBFileSpec sp) {
    if (!sp.IsValid())
        return fs::path();

    const char * dir = sp.GetDirectory();
    const char * file = sp.GetFilename();

    if (file == nullptr)
        return fs::path();

    if (dir == nullptr)
        return fs::path(file);

    fs::path res{dir};
    res /= file;
    return res;
}


std::filesystem::path lldb_file_spec_to_path(const lldb_private::FileSpec & sp) {
    auto dir = sp.GetDirectory();
    auto file = sp.GetFilename();

    if (file.IsEmpty()) {
        return {};
    }

    if (dir.IsEmpty()) {
        return {file.GetStringRef().str()};
    }

    fs::path res{dir.GetStringRef().str()};
    res /= file.GetStringRef().str();
    return res;
}


static std::string extract_func_name_at_address(lldb::SBAddress addr) {

    const char * fname = nullptr;

    // first trying extract inlined function info
    lldb::SBBlock block = addr.GetBlock();
    if (block.IsInlined()) {
        fname = block.GetInlinedName();
        if (fname != nullptr) {
            return fname;
        }
    }

    lldb::SBFunction func = addr.GetFunction();
    if (func.IsValid()) {
        fname = func.GetName();

        if (fname != nullptr &&
            // sometimes compiler generates 'operator()' for function
            // name. At this case we should try use symbol name.
            // We check that string starts with "operator()"
            // for case of template operator()
            ::strncmp(fname, "operator()", 10) != 0) {

            return fname;
        }
    }

    // try use symbol name instead of function name
    auto sym = addr.GetSymbol();
    if (sym.IsValid()) {
        const char * sname = sym.GetName();
        if (sname != nullptr) {
            return sname;
        }
    }

    // can't get symbol name. Use function name if available

    if (fname == nullptr) {
        return {};
    }

    return fname;
}


std::string func_name_at_address(const code_model & cm, lldb::SBAddress addr) {
    auto fname = extract_func_name_at_address(addr);
    if (fname.empty()) {
        return {};
    }

    // trying remove return type and parameters from function name
    std::string fname_no_ret;
    std::string fname_no_pars;
    if (cm.parse_function_name(fname, fname_no_ret, fname_no_pars)) {
        return fname_no_pars;
    }

    return fname;
}

}
