// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file file_document.cpp
/// Contains implementation of the file_document class

#include "file_document.hpp"
#include <sstream>


namespace cxxdbg {


void file_document::set_path(const file_document::path_t & p) {
    path_ = p;
    name_changed()();
}


std::wstring file_document::name_desc() const {
    std::wostringstream str;
    str << name();

    if (changed()) {
        str << "*";
    }

    return str.str();
}


}
