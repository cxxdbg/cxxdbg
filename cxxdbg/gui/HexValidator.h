// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file HexValidator.h
/// Contains definition of HexValidator class.

#pragma once

#include <QValidator>


namespace cxxdbg::gui {


/// \class HexValidator
/// Input validator for hex numbers
class HexValidator: public QValidator {
    Q_OBJECT

public:
    /// Validates input
    virtual State validate(QString & input, int & pos) const;
};


}
