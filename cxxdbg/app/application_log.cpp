// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file application_log.cpp
/// Contains implementation of the application_log class.

#include "application_log.hpp"
#include "cxxdbg/log/log_init.hpp"
#include <boost/log/expressions.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>


namespace bl = boost::log;
namespace bls = bl::sinks;
namespace ble = bl::expressions;


namespace cxxdbg {


/// Boost Log library sink backend implementation that redirects log records to
/// application_log_object
class application_log_sink_backend :
    public bls::basic_sink_backend <
        bls::combine_requirements <
            bls::synchronized_feeding,
            bls::formatted_records
        >::type
    >
{
public:
    using char_type = char;

    explicit application_log_sink_backend(application_log & app_log):
        app_log_{app_log} {}

    // The function consumes the log records that come from the frontend
    void consume(const bl::record_view & rec, const std::string & fmt_rec) {
        app_log_.on_log_record_received(fmt_rec);
    }

private:
    application_log & app_log_;
};



application_log::application_log(const std::string & ignore_cat) {
    // constructing sink for this application_log object
    auto backend = boost::make_shared<application_log_sink_backend>(*this);
    sink_ = boost::make_shared<sink_t>(backend);

    log::setup_sink_formatting(*sink_);

    if (!ignore_cat.empty()) {
        sink_->set_filter(ble::attr<std::string>("Category") != ignore_cat);
    }

    // adding sink into log core
    bl::core::get()->add_sink(sink_);
}


application_log::~application_log() {
    bl::core::get()->remove_sink(sink_);
}


void application_log::on_log_record_received(const std::string & rec) {
    old_records_.push_back(rec);
    if (old_records_.size() > 1000) {
        old_records_.pop_front();
    }

    record_received()(rec);
}


}
