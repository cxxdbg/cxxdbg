// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file record_type_impl.cpp
/// Contains implementation of the record_type_impl class.

#include "record_type_impl.hpp"
#include "target_base.hpp"
#include "type_utils.hpp"
#include "utils.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecordLayout.h>
#include <Plugins/TypeSystem/Clang/TypeSystemClang.h>
#include <lldb/Symbol/Function.h>
#include <lldb/Symbol/Type.h>
#include <Plugins/SymbolFile/DWARF/DWARFASTParserClang.h>
#include <ranges.hpp>
#include <sstream>
#include <iostream>


namespace cxxdbg::dbg::core {


record_type_impl::record_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
    auto tp = lldb_type();

    if (tp.GetTypeClass() != lldb::eTypeClassClass &&
        tp.GetTypeClass() != lldb::eTypeClassStruct &&
        tp.GetTypeClass() != lldb::eTypeClassUnion) {

        throw dbgfmt::backend::format_error{"lldb value type is not a class"};
    }
}


std::string record_type_impl::template_name() const {
    return lldb_type_template_name(lldb_type());
}


std::size_t record_type_impl::template_param_names_size() const {
    return lldb_type_template_params_size(lldb_type());
}


std::string record_type_impl::template_param_name(std::size_t index) const {
    auto nm = lldb_type_template_param(lldb_type(), index);
    if (nm.empty()) {
        std::ostringstream msg;
        msg << "can't find template parameter " << index << " in "
               "template '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    return nm;
}


size_t record_type_impl::template_params_size() const {
    return static_cast<size_t>(lldb_type().GetNumberOfTemplateArguments());
}


bool record_type_impl::template_param_is_type(size_t index) const {
    if (index >= template_params_size()) {
        std::ostringstream msg;
        msg << "invalid template parameter index " << index << " for type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    return lldb_type().GetTemplateArgumentKind(static_cast<uint32_t>(index)) == lldb::eTemplateArgumentKindType;
}


const type_impl * record_type_impl::template_param_type(size_t index) const {
    if (index >= template_params_size()) {
        std::ostringstream msg;
        msg << "invalid template parameter index " << index << " for type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    if (!template_param_is_type(index)) {
        std::ostringstream msg;
        msg << "template parameter at index " << index << " is not a type for type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    return targ().get_or_create_type(lldb_type().GetTemplateArgumentType(static_cast<uint32_t>(index)));
}


// Recursively checks if CXX clang record or it's base classes have virtual methods
// or virtual inheritance 
bool clang_record_has_virtual_methods_or_inheritance(clang::CXXRecordDecl * rec) {
    // checking if cxx record has virtual methods
    for (auto && method : rec->methods()) {
        if (method->isVirtual()) {
            return true;
        }
    }

    // checking base classes
    for (auto && base : rec->bases()) {
        if (base.isVirtual()) {
            return true;
        }

        auto base_rec = base.getType()->getAsCXXRecordDecl();
        if (base_rec && clang_record_has_virtual_methods_or_inheritance(base_rec)) {
            return true;
        }
    }

    return false;
}


bool record_type_impl::is_dynamic() const {
    auto rec = clang_cxx_record();
    if (!rec || !rec->hasDefinition()) {
        return false;
    }

    return clang_record_has_virtual_methods_or_inheritance(rec);
}


std::size_t record_type_impl::bases_size() const {
    return lldb_type().GetNumberOfDirectBaseClasses();
}


record_type_impl * record_type_impl::base_type_at(std::size_t i) const {
    if (i >= bases_size()) {
        std::ostringstream msg;
        msg << "can't find base with index " << i << " in type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto tp = lldb_type();
    auto mem = tp.GetDirectBaseClassAtIndex(i);
    if (!mem.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get record base with specified index"};
    }

    auto mem_type = mem.GetType();
    if (!mem_type.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get record base type with specified index"};
    }

    mem_type = untypedef_type(mem_type);

    auto base = dynamic_cast<record_type_impl*>(targ().get_or_create_type(mem_type));
    if (!base) {
        std::ostringstream msg;
        msg << "base type '" << mem_type.GetName() << "' is not a record";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    return base;
}


uint64_t record_type_impl::base_offset_at(size_t i, uint64_t obj_addr) const {
    if (i >= bases_size()) {
        std::ostringstream msg;
        msg << "can't find base with index " << i << " in type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto clang_ctx = clang_context();
    auto & ast_ctx = clang_ctx->getASTContext();
    auto cxx_rec = clang_cxx_record();
    assert(cxx_rec && "can't get clang cxx record for record type");
    const clang::ASTRecordLayout & layout = ast_ctx.getASTRecordLayout(cxx_rec);

    auto base_it = cxx_rec->bases_begin();
    std::advance(base_it, i);
    auto base_decl = base_it->getType()->getAs<clang::RecordType>()->getDecl();
    auto base_rec_decl = llvm::cast<clang::CXXRecordDecl>(base_decl);

    if (!base_it->isVirtual()) {
        return layout.getBaseClassOffset(base_rec_decl).getQuantity() * 8;
    }

    clang::VTableContextBase * vtable_ctx = ast_ctx.getVTableContext();

    if (vtable_ctx) {
        // creating lldb ValueObject for value at specified address
        auto ctype = lldb_type().GetSP()->GetCompilerType(false);
        lldb_private::ExecutionContext exe_ctx{targ().lldb_targ().GetSP(), true};
        auto val_obj = lldb_private::ValueObject::CreateValueObjectFromAddress("addr_obj",
                                                                                obj_addr,
                                                                                exe_ctx,
                                                                                ctype);

        // trying read base offset from object vtable
        int32_t bit_offset = 0;
        if (lldb_private::GetVBaseBitOffset(*vtable_ctx,
                                            *val_obj,
                                            layout,
                                            cxx_rec,
                                            base_rec_decl,
                                            bit_offset)) {
            return static_cast<uint64_t>(static_cast<uint32_t>(bit_offset));
        }
    }

    return layout.getVBaseClassOffset(base_rec_decl).getQuantity() * 8;
}


std::size_t record_type_impl::fields_size() const {
    lldb::SBType tp = lldb_type();
    return static_cast<std::size_t>(tp.GetNumberOfFields());
}


std::string record_type_impl::field_name_at(std::size_t i) const {
    if (i >= fields_size()) {
        std::ostringstream msg;
        msg << "can't find field with index " << i << " in type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto mem = lldb_type().GetFieldAtIndex(i);
    if (!mem.IsValid()) {
        std::ostringstream msg;
        msg << "can't get lldb field at index " << i << " of record " << name();
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto nm_cstr = mem.GetName();
    if (nm_cstr == nullptr) {
        return {};
    }

    return nm_cstr;
}


dbgfmt::backend::source_position record_type_impl::field_def_pos_at(size_t i) const {
    if (i >= fields_size()) {
        std::ostringstream msg;
        msg << "can't find field with index " << i << " in type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto cxx_rec = clang_cxx_record();
    assert(cxx_rec && "can't get clang cxx record for record type");

    size_t idx = 0;
    for (clang::FieldDecl * field : cxx_rec->fields()) {
        if (idx == i) {
            auto clang_ctx = clang_context();
            auto dwarf_parser = clang_ctx->GetDWARFParser();
            auto dwarf_clang_parser = static_cast<DWARFASTParserClang*>(dwarf_parser);
            assert(dwarf_clang_parser != nullptr && "dwarf parser is null");
            auto decl = dwarf_clang_parser->GetFieldDecl(field);
            return {lldb_file_spec_to_path(decl.GetFile()), decl.GetLine()};
        }

        ++idx;
    }

    assert(false && "should not reach here");
    return {};
}


type_impl * record_type_impl::field_type_at(std::size_t i) const {
    if (i >= fields_size()) {
        std::ostringstream msg;
        msg << "can't find field with index " << i << " in type '" << name() << "'";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto mem = lldb_type().GetFieldAtIndex(i);
    if (!mem.IsValid()) {
        std::ostringstream msg;
        msg << "can't get lldb field at index " << i << " of record " << name();
        throw dbgfmt::backend::format_error{msg.str()};
    }

    auto mem_type = mem.GetType();
    if (!mem_type.IsValid()) {
        throw dbgfmt::backend::format_error{"can't get lldb record field type"};
    }

    return targ().get_or_create_type(mem_type);
}


uint64_t record_type_impl::field_offset_at(std::size_t i) const {
    auto clang_ctx = clang_context();
    auto cxx_rec = clang_cxx_record();
    assert(cxx_rec && "can't get clang cxx record for record type");
    const auto & record_layout = clang_ctx->getASTContext().getASTRecordLayout(cxx_rec);

    // getting field offset
    return record_layout.getFieldOffset(i);
}


uint64_t record_type_impl::field_size_at(size_t i) const {
    auto lldb_fld = lldb_type().GetFieldAtIndex(i);
    if (lldb_fld.IsBitfield()) {
        return lldb_fld.GetBitfieldSizeInBits();
    } else {
        return lldb_fld.GetType().GetByteSize() * 8;
    }
}


const type_impl * record_type_impl::find_field(const std::string & nm,
                                               uint64_t * offset,
                                               uint64_t * size,
                                               dbgfmt::backend::source_position * pos,
                                               bool check_bases) const {
    return find_field_impl(lldb_type(), nm, offset, size, pos, check_bases);
}


type_impl * record_type_impl::mem_fun_type(const std::string & nm) const {
    auto func = find_mem_fun(nm);
    if (!func.IsValid()) {
        return nullptr;
    }

    return targ().get_or_create_type(func.GetType());
}


uint64_t record_type_impl::mem_fun_addr(const std::string & nm) const {
    auto func = find_mem_fun(nm);
    if (!func.IsValid()) {
        return 0;
    }

    return func.GetStartAddress().GetLoadAddress(targ().lldb_targ());
}


std::vector<std::string> record_type_impl::mem_funcs() const {
    return lldb_type_mem_funcs(lldb_type());
}


lldb::SBFunction record_type_impl::find_mem_fun(const std::string & name) const {
    auto rec = clang_cxx_record();
    assert(rec && "can't get clang cxx record for record type");

    if (!rec->hasDefinition()) {
        return {};
    }

    // looking for all functions with specified name
    // TODO: check on large targets, may be very slow
    auto funcs = targ().lldb_targ().FindFunctions(name.c_str(), lldb::eFunctionNameTypeMethod);

    for (uint32_t i = 0, e = funcs.GetSize(); i < e; ++i) {
        auto ctx = funcs.GetContextAtIndex(i);
        auto func = ctx.GetFunction();

        // checking that function is valid
        if (!func.IsValid())
            continue;

        auto func_decl = func.get()->GetDeclContext();

        // checking function is c/c++
        auto type_system = func_decl.GetTypeSystem();
        if (!llvm::dyn_cast<lldb_private::TypeSystemClang>(type_system)) {
            continue;
        }

        // checking that function is method
        clang::DeclContext * dctx = (clang::DeclContext*)(func_decl.GetOpaqueDeclContext());
        clang::CXXMethodDecl * mdecl = llvm::dyn_cast<clang::CXXMethodDecl>(dctx);
        if (!mdecl) {
            continue;
        }

        // checking that method belongs to this record
        clang::DeclContext * record_dctx = mdecl->getDeclContext();
        if (rec != record_dctx)
            continue;

        // found member function with specified name
        return func;
    }

    return {};
}


const type_impl * record_type_impl::find_field_impl(const lldb::SBType & r,
                                                    const std::string & nm,
                                                    uint64_t * offset,
                                                    uint64_t * size,
                                                    dbgfmt::backend::source_position * pos,
                                                    bool check_bases) const {
    auto rec = r;
    auto cxx_rec = get_record_decl_for_type(rec);

    // searching for direct field or field in anonymous members in this record
    size_t idx = 0;
    std::shared_ptr<lldb_private::TypeSystemClang> clang_ctx;

    if (offset != nullptr || pos != nullptr) {
        clang_ctx = clang_context();
    }

    for (clang::FieldDecl * field : cxx_rec->fields()) {
        if (field->getName() == nm) {
            // found direct field

            auto lldb_fld = rec.GetFieldAtIndex(idx);

            if (offset != nullptr) {
                assert(clang_ctx != nullptr && "clang_ctx should not be null here");
                const auto & layout = clang_ctx->getASTContext().getASTRecordLayout(cxx_rec);
                *offset = layout.getFieldOffset(idx);
            }

            if (size != nullptr) {
                if (lldb_fld.IsBitfield()) {
                    *size = lldb_fld.GetBitfieldSizeInBits();
                } else {
                    *size = lldb_fld.GetType().GetByteSize() * 8;
                }
            }

            if (pos != nullptr) {
                assert(clang_ctx != nullptr && "clang_ctx should not be null here");
                auto dwarf_parser = clang_ctx->GetDWARFParser();
                auto dwarf_clang_parser = static_cast<DWARFASTParserClang*>(dwarf_parser);
                assert(dwarf_clang_parser != nullptr && "dwarf parser is null");
                auto decl = dwarf_clang_parser->GetFieldDecl(field);
                pos->set_file(lldb_file_spec_to_path(decl.GetFile()));
                pos->set_line(decl.GetLine());
            }

            assert(lldb_fld.IsValid() && "invalid lldb field");
            return targ().get_or_create_type(lldb_fld.GetType());
        } else if (field->getName().empty()) {
            // found anonymous member, looking for field in it

            auto lldb_fld = rec.GetFieldAtIndex(idx);
            assert(lldb_fld.IsValid() && "invalid lldb field");
            auto lldb_fld_type = lldb_fld.GetType();
            assert(lldb_fld_type.IsValid() && "invalid lldb field type");

            uint64_t nst_offs = 0;
            uint64_t * nst_offs_ptr = offset != nullptr ? &nst_offs : nullptr;
            auto nst_type = find_field_impl(lldb_fld_type,
                                            nm,
                                            nst_offs_ptr,
                                            size,
                                            pos,
                                            check_bases);

            if (!nst_type) {
                ++idx;
                continue;
            }

            if (offset != nullptr) {
                assert(clang_ctx != nullptr && "clang_ctx should not be null here");
                const auto & layout = clang_ctx->getASTContext().getASTRecordLayout(cxx_rec);
                *offset = layout.getFieldOffset(idx) + nst_offs;
            }

            return nst_type;
        }
        ++idx;
    }

    // searching for field in base classes
    for (size_t i = 0, e = rec.GetNumberOfDirectBaseClasses(); i < e; ++i) {
        auto base = rec.GetDirectBaseClassAtIndex(i);
        auto fld_type = find_field_impl(base.GetType(), nm, offset, size, pos, check_bases);
        if (fld_type) {
            if (offset) {
                *offset += base.GetOffsetInBits();
            }

            return fld_type;
        }
    }

    return nullptr;
}


std::shared_ptr<lldb_private::TypeSystemClang> record_type_impl::clang_context() const {
    // getting compiler type for this type
    auto comp_type = compiler_type();

    // checking that type is a clang type
    auto type_system = comp_type.GetTypeSystem();
    auto clang_type_system = type_system.dyn_cast_or_null<lldb_private::TypeSystemClang>();
    if (!clang_type_system) {
        std::ostringstream msg;
        msg << "type '" << name() << "' is not a clang type";
        throw dbgfmt::backend::format_error{msg.str()};
    }

    return clang_type_system;
}



lldb_private::CompilerType record_type_impl::compiler_type(const lldb::SBType & t) const {
    auto type = t;
    auto compiler_type = type.GetSP()->GetCompilerType(true);
    assert(compiler_type.IsValid() && "compiler type for record is invalid");
    return compiler_type;
}


lldb_private::CompilerType record_type_impl::compiler_type() const {
    return compiler_type(lldb_type());
}


clang::CXXRecordDecl * record_type_impl::clang_cxx_record() const {
    return get_record_decl_for_type(lldb_type());
}


}
