// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file HexValidator.cpp
/// Contains implementation of HexValidator class.

#include "HexValidator.h"


namespace cxxdbg::gui {


HexValidator::State HexValidator::validate(QString & input, int & pos) const {
    if (input.isEmpty())
        return Intermediate;

    for (auto ch : input) {
        // checking that character is ascii
        if (static_cast<int>(ch.unicode()) > 128)
            return Invalid;

        char c = ch.toLatin1();

        // checking that character is hex
        if (!(std::isdigit(c) ||
              c == 'a' || c == 'A' ||
              c == 'b' || c == 'B' ||
              c == 'c' || c == 'C' ||
              c == 'd' || c == 'D' ||
              c == 'e' || c == 'E' ||
              c == 'f' || c == 'F'))
            return Invalid;
    }

    return Acceptable;
}


}
