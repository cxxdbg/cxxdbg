// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file code_breakpoint.hpp
/// Contains definition of the code_breakpoint class.

#pragma once

#include "breakpoint_base_impl.hpp"
#include "breakpoint_location.hpp"
#include <lldb/API/SBBreakpoint.h>
#include <boost/iterator/iterator_adaptor.hpp>


namespace cxxdbg::dbg::core {


/// Extern declaration of explicit instantiation of the breakpoint_base class.
/// We need this because we can't refer to lldb functions directly
/// outside of cxxdbgcore library
extern template class breakpoint_base_impl<lldb::SBBreakpoint, breakpoint_id::type_t::code>;


/// \class breakpoint
/// Represents single code breakpoint
class code_breakpoint:
        public breakpoint_base_impl<lldb::SBBreakpoint, breakpoint_id::type_t::code> {
public:
    /// Type of iterator over breakpoint locations
    class const_locations_iterator;

    /// Constructors breakpoint with specified pointer to LLDB breakpoint
    code_breakpoint(target_base & t, const lldb::SBBreakpoint & lldb_bp);

    /// Destructor, destroys object
    virtual ~code_breakpoint();

    /// Returns number of breakpoint locations
    std::size_t locations_size() const;

    /// Returns breakpoint location with specified index
    breakpoint_location location(std::size_t i) const;

    /// Returns iterator pointing to the first breakpoint location
    const_locations_iterator locations_begin() const;

    /// Returns iterator pointing to the one past the last breakpoint location
    const_locations_iterator locations_end() const;

private:
    /// Removes breakpoint from target
    virtual void remove() override;

    target_base & targ_;             ///< Reference to target
};



class code_breakpoint::const_locations_iterator: public boost::iterator_adaptor <
    const_locations_iterator,
    std::size_t,
    breakpoint_location,
    boost::random_access_traversal_tag,
    breakpoint_location,
    std::size_t
> {
public:
    /// Constructor, makes iterator with specified reference to breakpoint and
    /// location index
    const_locations_iterator(const code_breakpoint & bp, std::size_t i);

    /// Dereferences iterator
    reference dereference() const;

private:
    const code_breakpoint & bp_;         ///< Reference to breakpoint
};


}
