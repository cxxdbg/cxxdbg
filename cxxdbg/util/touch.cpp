// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file touch.cpp
/// Contains implementation of the touch function

#include "touch.hpp"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>


namespace cxxdbg::util {


void touch(const std::filesystem::path & p) {
    auto f = fopen(p.string().c_str(), "a");
    if (!f) {
        std::ostringstream msg;
        msg << "can't open file " << p << " for writing";
        auto ec = boost::system::error_code{errno, boost::system::system_category()};
        throw boost::system::system_error{ec, msg.str()};
    }

    fclose(f);
}


}
