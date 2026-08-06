// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file break_base_group.cpp
/// Contains implementation of the break_base_group class.

#include "break_base_group.hpp"


namespace cxxdbg::dbg::cli {


break_base_group::break_base_group(breakpoint_type type,
                                   const std::string & item_name,
                                   exec_processor & exec_proc,
                                   break_processor & break_proc):
command_group{std::string{"A set of commands for manipulating "} + item_name + "s"},
item_type_{type},
item_name_{item_name} {

    assert(!item_name_.empty() && "Item name must not be empty");
    item_name_cap_ = item_name_;
    item_name_cap_[0] = std::toupper(item_name_cap_[0], std::locale());

    auto cmd_name = std::string{"A set of commands for manipulating custom "} +
            item_name_ + " commands";

// TODO: implement command command in application classes
//    reg_exec(exec_proc, "command", cmd_name);

    reg_list(break_proc);
    reg_delete(break_proc);
    reg_enable(break_proc);
    reg_disable(break_proc);
    reg_modify(break_proc);
}


void break_base_group::reg_exec(exec_processor & exec_proc,
                                const std::string & name,
                                const std::string & desc) {
    reg_cmd(name, exec_command::make(exec_proc, item_name_ + " " + name, desc));
}


void break_base_group::reg_delete(break_processor & proc) {
    auto cmd = make_po_command([&proc, this](const variables_map & vars, auto && h) {
        auto num = vars["id"].as<breakpoint_num>();
        proc.delete_breakpoint({item_type_, num}, [num, h, this](auto && res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "deleted " << item_name_ << " " << num;
                h(msg.str());
            } else {
                h(res.error());
            }
        });
    });

    cmd->set_desc_help("Delete " + item_name_);
    cmd->opt_desc().add_options()
            ("id,i", po_value<breakpoint_num>()->required(),
             (item_name_cap_ + " id").c_str());
    cmd->popt_desc().add("id", 1);
    reg_cmd("delete", cmd);
}


void break_base_group::reg_enable(break_processor & proc) {
    auto cmd = make_po_command([&proc, this](const variables_map & vars, auto && h) {
        auto num = vars["id"].as<breakpoint_num>();
        proc.enable_breakpoint({item_type_, num}, [num, h, this](auto && res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "enabled " << item_name_ << " " << num;
                h(msg.str());
            } else {
                h(res.error());
            }
        });
    });

    cmd->set_desc_help("Enable " + item_name_);
    cmd->opt_desc().add_options()
            ("id,i", po_value<breakpoint_num>()->required(),
             (item_name_cap_ + " id").c_str());
    cmd->popt_desc().add("id", 1);
    reg_cmd("enable", cmd);
}


void break_base_group::reg_disable(break_processor & proc) {
    auto cmd = make_po_command([&proc, this](const variables_map & vars, auto && h) {
        auto num = vars["id"].as<breakpoint_num>();
        proc.disable_breakpoint({item_type_, num}, [num, h, this](auto && res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "disabled " << item_name_ << " " << num;
                h(msg.str());
            } else {
                h(res.error());
            }
        });
    });

    cmd->set_desc_help("Disable " + item_name_);
    cmd->opt_desc().add_options()
            ("id,i", po_value<breakpoint_num>()->required(),
             (item_name_cap_ + " id").c_str());
    cmd->popt_desc().add("id", 1);
    reg_cmd("disable", cmd);
}


void break_base_group::reg_modify(break_processor & proc) {
    auto cmd = make_po_command([&proc, this](const variables_map & vars, auto && h) {

        // parsing breakpoint base options
        break_properties props;
        process_break_options(vars, props);

        // modifying breakpoint
        auto num = vars["id"].as<breakpoint_num>();
        proc.set_breakpoint_props({item_type_, num}, props, [h, num, this](auto && res) {
            if (res.is_ok()) {
                std::ostringstream msg;
                msg << "modified " << item_name_ << " " << num;
                h(msg.str());
            } else {
                h(res.error());
            }
        });
    });

    cmd->set_desc_help("Modify " + item_name_);
    cmd->opt_desc().add_options()
            ("id", po_value<breakpoint_num>()->required(),
             (item_name_cap_ + " id").c_str());
    add_break_options(item_name_cap_, cmd->opt_desc());
    cmd->popt_desc().add("id", 1);
    reg_cmd("modify", cmd);
}


void break_base_group::reg_list(break_processor & proc) {
    auto cmd = make_po_command([&proc, this](const variables_map &, auto && h) {
        h(proc.list_breakpoints(item_type_));
    });

    cmd->set_desc_help("List all " + item_name_mult_);
    reg_cmd("list", cmd);
}


}
