// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_platform_processor.hpp
/// Contains definition of the mock_platform_processor class.

#pragma once

#include "../platform_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock processor for platform commands group
class mock_platform_processor: public platform_processor,
                               public mock::object<mock_platform_processor> {
public:
    MOCK_DEFINE_METHOD_1(select_platform, bool (std::string_view))
    MOCK_DEFINE_METHOD_7(connect_to_platform, void (const std::string_view,
                                                    const std::string &,
                                                    bool enable_rsync,
                                                    const std::string &,
                                                    const std::string &,
                                                    bool,
                                                    const async::result_handler<std::string> &))
    MOCK_DEFINE_METHOD_0(disconnect_from_platform, void ())
};


}
