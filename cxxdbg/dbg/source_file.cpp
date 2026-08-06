// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file source_file.cpp
/// Contains implementation of source_file class.

#include "source_file.hpp"


namespace fs = std::filesystem;


namespace cxxdbg::dbg {


source_file::source_file(const fs::path & p):
path_(p) {
}


const fs::path & source_file::path() const {
    return path_;
}


std::wstring source_file::name() const {
    return path().filename().generic_wstring();
}


}
