// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debug_document_widget_factory.cpp
/// Contains implementation of the debug_document_widget_factory class.

#include "debug_document_widget_factory.hpp"
#include "cxxdbg/app/text_file_document.hpp"


namespace cxxdbg::gui {


std::unique_ptr<document_widget> debug_document_widget_factory::create_document_widget(document & doc) {
    // TODO: add support of other document types

    if (auto tdoc = dynamic_cast<text_model_file_document<true>*>(&doc)) {
        using widget_t = text_document_widget<text_model_file_document<true>::text_model_t,
                                              text_model_file_document<true>::selection_model_t,
                                              text_model_file_document<true>::controller_t>;
        return std::make_unique<widget_t>(tdoc, &delegate_);
    } else if (auto tdoc = dynamic_cast<text_model_file_document<false>*>(&doc)) {
        using widget_t = text_document_widget<text_model_file_document<false>::text_model_t,
                                              text_model_file_document<false>::selection_model_t,
                                              text_model_file_document<false>::controller_t>;
        return std::make_unique<widget_t>(tdoc, &delegate_);
    } else {
        assert(false && "unsupported document type opened");
        return {};
    }
}


}
