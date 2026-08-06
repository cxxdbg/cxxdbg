// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file Utils.h
/// Contains declarations of utility GUI functions

#pragma once

#include <QFont>


namespace cxxdbg::gui {


/// Returns default fixed width font
QFont defaultFixedFont();

/// Returns default code viewer font
QFont defaultCodeViewerFont();

/// Makes icon from svg images of multiple sizes
QIcon makeSvgIcon(const QString & path, const QString & name);


}
