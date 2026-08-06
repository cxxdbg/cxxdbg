// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file application_log.hpp
/// Contains definition of the application_log class.

#pragma once

#include "cxxdbg/util/signals.hpp"
#include <list>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/log/sinks/sync_frontend.hpp>


namespace cxxdbg {


class application_log_sink_backend;


/// Incapsulates boost based application log that sends signals
/// when new log records received
class application_log {
    friend class application_log_sink_backend;

public:
    /// Constructs application_log object and installs all required
    /// sinks into boost log. The ignore_cat is the optional name
    /// of the log category that should be ignored by log
    application_log(const std::string & ignore_cat = {});

    /// Destroys application_log object and removes all used sinks
    /// from boost log
    ~application_log();

    /// The signal is emitted when new log record arrives to log
    CXXDBG_DEFINE_SIGNALX(record_received, void(const std::string&))

    /// Returns range of old log records
    auto & old_records() const {
        return old_records_;
    }

private:
    /// Called when log sink receives new log record
    void on_log_record_received(const std::string & rec);

    std::list<std::string> old_records_;            ///< Old log records
    using sink_t = boost::log::sinks::synchronous_sink<application_log_sink_backend>;
    boost::shared_ptr<sink_t> sink_;
};


}


