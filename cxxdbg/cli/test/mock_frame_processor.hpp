// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_frame_processor.hpp
/// Contains definition of the mock_frame_processor class.

#pragma once

#include "cxxdbg/cli/frame_processor.hpp"
#include "cxxdbg/mock/mock.hpp"


namespace cxxdbg::dbg::cli::test {


/// Mock class for frame_processor abstract class.
class mock_frame_processor:
        public frame_processor,
        public mock::object<mock_frame_processor> {

public:
    MOCK_DEFINE_METHOD_2(select_frame, void (std::size_t,
                                             const async::result_handler<> &))
};


}
