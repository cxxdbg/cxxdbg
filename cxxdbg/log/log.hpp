// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file log.hpp
/// Contains definition of APP category log singleton and macroses

#pragma once

// Check that BOOST_LOG_DYN_LINK or BOOST_LOG_NO_DYN_LINK is defined.
// This is needed to ensure all log depended code links the cxxdbglog library
// that sets correct preprocessor definitions for dynamic/static linking
// of the boost log library
#if !defined(BOOST_LOG_DYN_LINK) && !defined(BOOST_LOG_NO_DYN_LINK)
#error "log.hpp is included without linking with cxxdbglog library"
#endif


#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>


namespace cxxdbg { namespace log {


using logger_t = boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>;

/// Returns reference to logger
inline logger_t & get_logger() {
    static logger_t logger;
    return logger;
}


#define CXXDBG_LOG(cat, level) \
    BOOST_LOG_SEV(::cxxdbg::log::get_logger(), level) \
        << ::boost::log::add_value("Category", #cat)

#define CXXDBG_LOG_TRACE(cat)     CXXDBG_LOG(cat, ::boost::log::trivial::trace)
#define CXXDBG_LOG_DEBUG(cat)     CXXDBG_LOG(cat, ::boost::log::trivial::debug)
#define CXXDBG_LOG_INFO(cat)      CXXDBG_LOG(cat, ::boost::log::trivial::info)
#define CXXDBG_LOG_WARNING(cat)   CXXDBG_LOG(cat, ::boost::log::trivial::warning)
#define CXXDBG_LOG_ERROR(cat)     CXXDBG_LOG(cat, ::boost::log::trivial::error)
#define CXXDBG_LOG_FATAL(cat)     CXXDBG_LOG(cat, ::boost::log::trivial::fatal)


#define CXXDBG_LOG_SCAT(cat, scat, level) \
    BOOST_LOG_SEV(::cxxdbg::log::get_logger(), level) \
        << ::boost::log::add_value("Category", #cat) \
        << ::boost::log::add_value("Subcategory", #scat)

#define CXXDBG_LOG_SCAT_TRACE(cat, scat)      CXXDBG_LOG_SCAT(cat, scat, ::boost::log::trivial::trace)
#define CXXDBG_LOG_SCAT_DEBUG(cat, scat)      CXXDBG_LOG_SCAT(cat, scat, ::boost::log::trivial::debug)
#define CXXDBG_LOG_SCAT_INFO(cat, scat)       CXXDBG_LOG_SCAT(cat, scat, ::boost::log::trivial::info)
#define CXXDBG_LOG_SCAT_WARNING(cat, scat)    CXXDBG_LOG_SCAT(cat, scat, ::boost::log::trivial::warning)
#define CXXDBG_LOG_SCAT_ERROR(cat, scat)      CXXDBG_LOG_SCAT(cat, scat, ::boost::log::trivial::error)
#define CXXDBG_LOG_SCAT_FATAL(cat, scat)      CXXDBG_LOG_SCAT(cat, scat, ::boost::log::trivial::fatal)


} }


