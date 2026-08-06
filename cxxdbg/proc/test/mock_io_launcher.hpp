// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock_io_launcher.hpp
/// Contains definition of the mock_io_launcher class.

#pragma once

#include "../io_launcher.hpp"
#include "cxxdbg/mock/mock.hpp"
#include "cxxdbg/proc/launch_parameters.hpp"


namespace cxxdbg::proc::test {


/// Mock process launcher implementation
class mock_io_launcher: public mock::object<mock_io_launcher>,
                        virtual public io_launcher {
public:
    class child: public mock::object<child>,
                 virtual public child_io_monitor {
    public:
        MOCK_DEFINE_METHOD_0(terminate, void())
        MOCK_DEFINE_METHOD_1(send_stdin, void(const std::string&))
        MOCK_DEFINE_METHOD_0(close_stdin, void())
    };

    MOCK_DEFINE_METHOD_1(launch_io, std::unique_ptr<child_io_monitor> (const proc::launch_parameters &))
};


}


