// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxSyntaxHighlighter.h
/// Contains definition of the CxxSyntaxHighlighter class.

#pragma once

#include <QSyntaxHighlighter>


namespace cxxdbg::gui {


/// C++ syntax highlighter
class CxxSyntaxHighlighter: public QSyntaxHighlighter {
public:
    /// Constructor, makes c++ syntax highlighter and installs
    /// it in specified text document
    CxxSyntaxHighlighter(QTextDocument * doc);

protected:
    /// Highlights block of text
    void highlightBlock(const QString & text) override;

private:
};


}
