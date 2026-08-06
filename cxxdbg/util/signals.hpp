// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file signals.hpp
/// Contains definition of types for handling signals

#pragma once

#include <boost/signals2/signal.hpp>


namespace cxxdbg { namespace util {


/// Connection to a signal
typedef boost::signals2::connection signal_connection;

/// Scoped connection to a signal
typedef boost::signals2::scoped_connection scoped_signal_connection;

/// Type of signal
template <typename Func> using signal = boost::signals2::signal<Func>;


/// Defines new signal
#define CXXDBG_DEFINE_SIGNAL(name, proto) \
    public: \
        boost::signals2::connection connect_##name(const std::function<proto> & handler) { \
            return name##_.connect(handler); \
        } \
    \
    private: \
        boost::signals2::signal<proto> name##_; \
    public:


/// Defines new signal
#define CXXDBG_DEFINE_SIGNALX(name, proto) \
    public: \
        auto & name() const { return name##_; } \
    private: \
        mutable boost::signals2::signal<proto> name##_; \
    public:

/// Defines new slot
#define CXXDBG_DEFINE_SLOTX(name, proto) \
    public: \
        auto & name() const { return name##_; } \
    private: \
        mutable std::function<proto> name##_; \
    public:

} }

