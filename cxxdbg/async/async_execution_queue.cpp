// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_execution_queue.cpp
/// Contains implementation of async_exection_queue class and related classes.

#include "async_execution_queue.hpp"
#include <cassert>


namespace cxxdbg { namespace async {


execution_queue::execution_queue(event_queue & equeue, event_queue & hqueue):
exec_queue_(equeue),
handle_queue_(hqueue) {
}


execution_queue::~execution_queue() {
}


} }
