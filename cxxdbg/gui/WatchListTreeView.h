// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file WatchListTreeView.h
/// Contains definition of the WatchListTreeView

#pragma once

#include "CxxdbgTreeView.h"
#include "cxxdbg/app/signals.hpp"
#include <cstddef>


namespace cxxdbg::dbg {
    class debugger;
    class watch_model;
}


class QAbstractItemModel;


namespace cxxdbg::gui {


/// Tree view which displays list from specified model.
/// Contain "hexadecimal display" item in context menu
class WatchListTreeView: public CxxdbgTreeView {
public:
    /// Constructor, makes watch tree view with specified name, reference
    /// to watch model and Qt model
    WatchListTreeView(const QString & name,
                      cxxdbg::dbg::debugger & dbg,
                      SortableAppRoItemModel & qtModel);

private:
    /// Connection to format options changed signal
    cxxdbg::scoped_signal_connection fmt_opts_changed_con_;
};


}
