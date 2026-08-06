// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file result.cpp
/// Contains implementation of the result class and related classes.

#include "result.hpp"


namespace cxxdbg { namespace async {


result_base::result_base():
is_ok_{true} {
}


result_base::result_base(const std::string & err):
is_ok_(false),
err_(err) {
}


bool result_base::is_ok() const {
    return is_ok_;
}


const std::string & result_base::error() const {
    assert(!is_ok() && "Invalid result status");
    return err_;
}


void result_base::set_error(const std::string & err) {
    is_ok_ = false;
    err_ = err;
}


void result_base::set_ok() {
    is_ok_ = true;
    err_.erase();
}


} }
