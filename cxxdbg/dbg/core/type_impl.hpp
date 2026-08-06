// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file type_impl.hpp
/// Contains definition of the type_impl class.

#pragma once

#include "dbgfmt/type_impl_common.hpp"
#include <lldb/API/SBType.h>


namespace cxxdbg::dbg::core {


class target_base;
class value_impl;


/// Implenentation of value type in core
class type_impl: public std::enable_shared_from_this<type_impl>,
                 virtual public dbgfmt::type_impl_common {

    friend class target_base;
    friend class value_impl;

public:
    /// Contructs value type implementation with specified reference to
    /// target and lldb type
    type_impl(target_base & targ, const lldb::SBType & t);

    /// Returns pointer to value context impl, i. e. pointer to target
    dbgfmt::backend::type_context_impl * ctx() const override;

    /// Returns name of type of value
    std::string name() const override;

    /// Returns source position of definition of type. Returns invalid
    /// source position if not available
    dbgfmt::backend::source_position def_pos() const override;

    /// Returns qualified name of type of value
    std::string qual_name() const override;

    /// Returns type implementation representing pointer to this type
    const type_impl * pointer_type() const override;

    /// Returns reference to target for that type is defined
    auto & targ() const { return targ_; }

    /// Returns type size
    size_t size() const override;

    /// Tries detect dynamic type from this static type at specified address
    const type_impl * dynamic_type_at_addr(uint64_t & addr) const;

protected:
    /// Returns copy of lldb type
    lldb::SBType lldb_type() const;

private:
    target_base & targ_;                 ///< Reference to target
    lldb::SBType lldb_type_;
};


using type_impl_sp = std::shared_ptr<type_impl>;


}
