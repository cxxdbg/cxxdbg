// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file forward.hpp
/// Contains forward declarations of classes from async library

#pragma once

#include <functional>


namespace cxxdbg { namespace async {


template <typename T = void> class result;
class result_base;
class event_queue;
class execution_queue;

template <typename T = void> using result_handler = std::function<void (const result<T> &)>;


} }


