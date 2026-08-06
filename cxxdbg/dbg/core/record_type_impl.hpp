// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file record_type_impl.hpp
/// Contains definition of the record_type_impl class.

#pragma once

#include "type_impl.hpp"
#include "dbgfmt/backend/record_type_impl.hpp"
#include <clang/AST/DeclCXX.h>
#include <Plugins/TypeSystem/Clang/TypeSystemClang.h>


namespace cxxdbg::dbg::core {


/// Implementation of the dbgfmt::backend::record_type_impl interface for core
class record_type_impl: virtual public dbgfmt::backend::record_type_impl, public type_impl {
public:
    /// Constructs record type implementation with specified reference to target
    /// and pointer to lldb type
    record_type_impl(target_base & targ, const lldb::SBType & tp);


    /// Returns name of template class without template parameters
    std::string template_name() const override;

    /// Returns number of template parameters
    std::size_t template_param_names_size() const override;

    /// Returns template parameter with specified index
    std::string template_param_name(std::size_t index) const override;

    /// Returns number of template parameters
    size_t template_params_size() const override;

    /// Returns true if template parameter is a type
    bool template_param_is_type(size_t index) const override;

    /// Returns template parameter type
    const core::type_impl * template_param_type(size_t index) const override;

    /// Returns true if record is dynamic type (has vtable)
    bool is_dynamic() const override;


    /// Returns number of direct base classes
    std::size_t bases_size() const override;

    /// Returns type of direct base class at specified index
    record_type_impl * base_type_at(std::size_t i) const override;

    /// Returns offset of base class value at specified index
    uint64_t base_offset_at(size_t i, uint64_t obj_addr) const override;


    /// Returns number of fields
    std::size_t fields_size() const override;

    /// Returns name of field with specified index
    std::string field_name_at(std::size_t i) const override;

    /// Returns source position of definition of record field at
    /// specified index. Returns invalid source position if not avaliable.
    dbgfmt::backend::source_position field_def_pos_at(size_t i) const override;

    /// Returns value of field with specified index
    core::type_impl * field_type_at(std::size_t i) const override;

    /// Returns offset of field in bits with specified index
    uint64_t field_offset_at(size_t i) const override;

    /// Returns size in bits of field with specified index
    uint64_t field_size_at(size_t i) const override;

    /// Gets type, offset in bits, size in bits, and definition position
    /// of field with specified name. Returns nullptr if field was not found
    const core::type_impl * find_field(const std::string & nm,
                                 uint64_t * offset,
                                 uint64_t * size,
                                 dbgfmt::backend::source_position * pos,
                                 bool check_bases) const override;

    /// Rerurns type of member function with specified name, or 0 if function
    /// with such name not found
    core::type_impl * mem_fun_type(const std::string & nm) const override;

    /// Returns address of member function with specified name, or 0 if
    /// address of function not found
    uint64_t mem_fun_addr(const std::string & nm) const override;

    /// Returns vector of names of member functions of record
    std::vector<std::string> mem_funcs() const override;

private:
    /// Returns lldb function for member function with specified name
    lldb::SBFunction find_mem_fun(const std::string & name) const;

    /// Recursive implementation of the find_field function
    const core::type_impl * find_field_impl(const lldb::SBType & r,
                                      const std::string & nm,
                                      uint64_t * offset,
                                      uint64_t * size,
                                      dbgfmt::backend::source_position * pos,
                                      bool check_bases) const;

    /// Returns clang AST context for this lldb type
    std::shared_ptr<lldb_private::TypeSystemClang> clang_context() const;

    /// Returns lldb compiler type for lldb type
    lldb_private::CompilerType compiler_type(const lldb::SBType & type) const;

    /// Returns lldb compiler type for this lldb type
    lldb_private::CompilerType compiler_type() const;

    /// Returns CXX record decl for this lldb type
    clang::CXXRecordDecl * clang_cxx_record() const;
};


}
