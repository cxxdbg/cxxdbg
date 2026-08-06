// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file type_utils.hpp
/// Contains declarations of utility functions for working with
/// lldb types

#pragma once

#include <clang/AST/DeclCXX.h>
#include <lldb/API/SBType.h>
#include <vector>


namespace cxxdbg::dbg::core {


/// Returns true if lldb type is record (class / struct / union)
bool lldb_type_is_record(lldb::SBType type);

/// Returns type of pointee for lldb pointers/references
lldb::SBType lldb_type_pointee(lldb::SBType type);

/// Returns unqualified name of lldb type
std::string lldb_type_name(lldb::SBType type);

/// Returns qualified name of lldb type
std::string lldb_type_qual_name(lldb::SBType type);

/// Returns name of template class for lldb type.
/// Extracts template name from full type name string
std::string lldb_type_template_name(lldb::SBType type);

/// Returns number of template parameters for lldb type.
/// Extracts template parameters from full type name string
size_t lldb_type_template_params_size(lldb::SBType type);

/// Returns name/value of template parameter with specified index
/// for lldb type. Extracts template parameter names from
/// full type name string. Return empty string on error.
std::string lldb_type_template_param(lldb::SBType type, size_t index);

/// Returns clang record decl for lldb type.
/// Returns nullptr if record type can't be retrieved
clang::CXXRecordDecl * get_record_decl_for_type(lldb::SBType type);

/// Returns vector of names of all member functions of
/// specified record type
std::vector<std::string> lldb_type_mem_funcs(lldb::SBType type);

/// Untypedefs type. Returns first non typedef type in typedef chain
lldb::SBType untypedef_type(lldb::SBType type);


}
