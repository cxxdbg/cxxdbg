// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debug_document_widget_factory.hpp
/// Cotnains definition of the debug_document_widget_factory class.

#pragma once

#include "document_widget.hpp"
#include "debug_source_code_widget_delegate.hpp"


namespace cxxdbg::gui {


/// Factory for creating document widgets for debugger application
class debug_document_widget_factory: public document_widget_factory {
public:
    /// Constructs factory with specified reference to debugger and debug ui
    debug_document_widget_factory(cxxdbg::dbg::debugger & dbg, DebugUI & dbgUi):
        delegate_{dbg, dbgUi} {}

    /// Creates document widget for specified document
    std::unique_ptr<document_widget> create_document_widget(cxxdbg::document & doc) override;

private:
    debug_source_code_widget_delegate delegate_;
};


}
