// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file cxxdbg_connect.hpp
/// Contains definition of the cxxdbg_connect function.s

#pragma once


namespace cxxdbg::gui {


void cxxdbg_connect_handle_error(const std::exception & err);
void cxxdbg_connect_handle_unknown_error();


template <typename Handler, typename ... Args>
void cxxdbg_connect_handler(const Handler & handler, Args && ... args) {
    try {
        if constexpr (std::is_invocable_v<Handler, Args...>) {
            handler(std::forward<Args>(args)...);
        } else {
            // special case for omitted parameters
            handler();
        }
    }
    catch (std::exception & err) {
        cxxdbg_connect_handle_error(err);
    }
    catch (...) {
        cxxdbg_connect_handle_unknown_error();
    }
}


/// Establishes connection to a qt signal. This functions should be used instead of
/// QObject::connect function everythere because it supports exception handling in slots.
template <typename Sender, typename Slot, typename Handler>
auto cxxdbg_connect(Sender * sender, Slot && slot, const Handler & handler) {
    return QObject::connect(sender, std::forward<Slot>(slot), [handler](auto && ... args) {
        cxxdbg_connect_handler(handler, std::forward<decltype(args)>(args)...);
    });
}


}
