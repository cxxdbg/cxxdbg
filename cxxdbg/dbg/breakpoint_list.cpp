// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file breakpoint_list.cpp
/// Contains implementation of the breakpoint_list class.

#include "breakpoint_list.hpp"
#include "address_breakpoint.hpp"
#include "breakpoint_list_impl.hpp"
#include "breakpoint_location.hpp"
#include "breakpoint_utils.hpp"
#include "exception_thrown_breakpoint.hpp"
#include "exception_caught_breakpoint.hpp"
#include "function_breakpoint.hpp"
#include "source_position_breakpoint.hpp"
#include "stack_frame.hpp"
#include "target.hpp"
#include "watchpoint.hpp"
#include <ranges.hpp>


namespace cxxdbg::dbg {


void breakpoint_list::add_breakpoint(const source_position_info & pos,
                                     const code_breakpoint_handler & h,
                                     const source_position & toggle_pos) {
    auto bp = std::make_shared<source_position_breakpoint>(src_model_, next_bp_num(), pos);
    add_code_breakpoint(bp, h, toggle_pos);
}


void breakpoint_list::add_breakpoint(const std::string & func_name, const code_breakpoint_handler & h) {
    auto bp = std::make_shared<function_breakpoint>(src_model_, next_bp_num(), func_name);
    add_code_breakpoint(bp, h);
}


void breakpoint_list::add_breakpoint(std::uint64_t addr, const code_breakpoint_handler & h) {
    auto bp = std::make_shared<address_breakpoint>(src_model_, next_bp_num(), addr);
    add_code_breakpoint(bp, h);
}


void breakpoint_list::add_breakpoint_exception_thrown(const code_breakpoint_handler & h) {
    auto bp = std::make_shared<exception_thrown_breakpoint>(src_model_, next_bp_num());
    add_code_breakpoint(bp, h);
}


void breakpoint_list::add_breakpoint_exception_caught(const code_breakpoint_handler &h) {
    auto bp = std::make_shared<exception_caught_breakpoint>(src_model_, next_bp_num());
    add_code_breakpoint(bp, h);
}


void breakpoint_list::add_var_watchpoint(const std::string & name,
                                         bool read,
                                         bool write,
                                         size_t size,
                                         const watchpoint_handler & handl) {
    impl_->add_var_watchpoint(name, read, write, size, [this, handl, read, write, size, name](auto && res) {
        if (!res.is_ok()) {
            handl({res.error()});
        } else {
            auto wp = std::make_shared<variable_watchpoint>(res.value()->num(), read, write, size, name);
            breakpoints_.emplace(bp_id(wp.get()), wp);
            added()(wp.get());
            list_changed()();

            handl({wp.get()});
        }
    });
}


void breakpoint_list::add_expr_watchpoint(const std::string & expr,
                                          bool read,
                                          bool write,
                                          size_t size,
                                          const watchpoint_handler & handl) {
    impl_->add_expr_watchpoint(expr, read, write, size, [this, handl, read, write, size, expr](auto && res) {
        if (!res.is_ok()) {
            handl({res.error()});
        } else {
            auto wp = std::make_shared<expression_watchpoint>(res.value()->num(), read, write, size, expr);
            breakpoints_.emplace(bp_id(wp.get()), wp);
            added()(wp.get());
            list_changed()();

            handl({wp.get()});
        }
    });
}


void breakpoint_list::remove_breakpoint(const breakpoint * bp) {

    // removing breakpoint object from list
    auto it = breakpoints_.find(bp_id(bp));
    assert(it != breakpoints_.end() && "Can't find breakpoint in the list of breakpoints");
    breakpoint_sp bpp = it->second;
    breakpoints_.erase(it);

    // emitting signals
    removed()(bpp.get());
    list_changed()();

    // removing breakpoint from impl
    if (impl_) {
        impl_->remove_breakpoint(bp_id(bp));
    }
}


void breakpoint_list::enable_breakpoint(const breakpoint * bp) {

    // setting flag in breakpoint object
    // TODO remove const qualifier from bp and use it
    auto it = breakpoints_.find(bp_id(bp));
    assert(it != breakpoints_.end() && "Can't find breakpoint");
    it->second->set_enabled(true);

    // emitting signals
    changed()(bp);
    list_changed()();

    // enabling breakpoint in implementation
    if (impl_) {
        impl_->enable_breakpoint(bp_id(bp));
    }
}


void breakpoint_list::disable_breakpoint(const breakpoint * bp) {
    // setting flag in breakpoint object
    // TODO remove const qualifier from bp and use it
    auto it = breakpoints_.find(bp_id(bp));
    assert(it != breakpoints_.end() && "Can't find breakpoint");
    it->second->set_enabled(false);

    // emitting signals
    changed()(bp);
    list_changed()();

    // disabling breakpoint in implementation
    if (impl_) {
        impl_->disable_breakpoint(bp_id(bp));
    }
}


void breakpoint_list::remove_all_breakpoints() {
    while (!std::ranges::empty(all())) {
        auto all_bps = all();
        remove_breakpoint(*std::ranges::begin(all_bps));
    }
}


void breakpoint_list::enable_all_breakpoints() {
    for (auto && bp : all()) {
        enable_breakpoint(bp);
    }
}


void breakpoint_list::disable_all_breakpoints() {
    for (auto && bp : all()) {
        disable_breakpoint(bp);
    }
}


void breakpoint_list::set_breakpoint_condition(const breakpoint * bp, const std::string & cond) {
    // setting condition in breakpoint object
    auto it = breakpoints_.find(bp_id(bp));
    assert(it != breakpoints_.end() && "Can't find breakpoint");
    it->second->set_condition(cond);

    // emitting changed signals
    changed()(bp);
    list_changed()();

    // setting breakpoint condition in implementation
    if (impl_) {
        impl_->set_breakpoint_condition(bp_id(bp), cond);
    }
}


void breakpoint_list::set_breakpoint_hit_count(const breakpoint * bp,
                                               bool hit_count_enabled,
                                               unsigned int hit_count,
                                               bool reset_hit_count) {
    // setting hit count fields in breakpoint object
    auto it = breakpoints_.find(bp_id(bp));
    assert(it != breakpoints_.end() && "Can't find breakpoint");
    it->second->set_hit_count_enabled(hit_count_enabled);
    it->second->set_hit_count(hit_count);
    if (reset_hit_count) {
        it->second->reset_curr_hit_count();
    }

    // sending breakpoint signals
    changed()(it->second.get());
    list_changed()();

    // setting breakpoint ignore count in impl
    if (impl_) {
        auto ignore_count = it->second->ignore_count();
        impl_->set_breakpoint_ignore_count(bp_id(bp), ignore_count);
    }
}


const code_breakpoint * breakpoint_list::find_code_breakpoint(breakpoint_num n) const {
    auto bp = find_breakpoint({breakpoint_type::watch, n});
    if (bp == nullptr) {
        return nullptr;
    }

    auto cbp = dynamic_cast<const code_breakpoint*>(bp);
    assert(cbp && "Invalid breakpoint type");
    return cbp;
}


const watchpoint * breakpoint_list::find_watchpoint(breakpoint_num n) const {
    auto bp = find_breakpoint({breakpoint_type::watch, n});
    if (bp == nullptr) {
        return nullptr;
    }

    auto wp = dynamic_cast<const watchpoint*>(bp);
    assert(wp && "Invalid breakpoint type");
    return wp;
}


const breakpoint * breakpoint_list::find_breakpoint(const breakpoint_id & id) const {
    auto it = breakpoints_.find(id);
    if (it == breakpoints_.end()) {
        return nullptr;
    }

    return it->second.get();
}


void breakpoint_list::add_breakpoint_locations(code_breakpoint * bp,
                                               const code_breakpoint_info & bp_inf) {

    for (auto && loc_info : bp_inf.locations()) {
        // resolving source file in source position
        code_position pos = target::convert_code_position_info(src_model_, loc_info.pos());

        std::shared_ptr<breakpoint_location> loc(new breakpoint_location(bp, loc_info.id(), pos));
        bp->add_location(loc);
    }
}


breakpoint_list::num_t breakpoint_list::next_bp_num() {
    return next_bp_num_++;
}


breakpoint_list::num_t breakpoint_list::next_wp_num() {
    return next_wp_num_++;
}


void breakpoint_list::reset_breakpoints() {
    // calling before_start for all breakpoints
    for (auto && pair : breakpoints_) {
        auto bp = pair.second.get();

        bp->before_start();
        changed()(bp);

        unsigned int ignore_count = 0;
        if (bp->hit_count_enabled()) {
            unsigned int hit_count = bp->hit_count();
            if (hit_count > 0) {
                ignore_count = hit_count - 1;
            }
        }

        impl_->set_breakpoint_ignore_count(bp_id(bp), ignore_count);
    }

    list_changed()();
}


void breakpoint_list::update_breakpoints(const std::vector<std::shared_ptr<breakpoint_info>> & bps,
                                         bool is_stop_reason_wp,
                                         const stack_frame * curr_frame) {
    // updating breakpoints
    // TODO remove n^2 complexity
    bool l_changed = false;
    for (auto bp : breakpoints_) {
        for (auto bpinf : bps) {
            if (bp.first != bpinf->id()) {
                continue;
            }

            bool bp_changed = false;

            bp_changed |= bp.second->update(bpinf.get());

            auto * wp = dynamic_cast<watchpoint *>(bp.second.get());
            if (wp && is_stop_reason_wp && curr_frame != nullptr) {
                auto pos = curr_frame->pos().src_pos();
                if (wp->get_source_position() != pos) {
                    wp->set_position(pos);
                    l_changed = true;
                }
            }

            if (bp_changed) {
                changed()(bp.second.get());
                l_changed = true;
            }

            break;
        }
    }

    if(l_changed) {
        list_changed()();
    }
}

void breakpoint_list::toggle_breakpoint(const source_file * src, unsigned int line) {
    // getting breakpoints at current position
    auto bps = breakpoints_at_line(*this, {src, line});
    auto bps_list = std::list<const code_breakpoint*>{std::ranges::begin(bps), std::ranges::end(bps)};

    if (!bps_list.empty()) {
        // removing all breakpoints

        while (!bps_list.empty()) {
            remove_breakpoint(bps_list.front());
            bps_list.pop_front();
        }
        return;
    }

    // adding breakpoint at position
    source_position_info pos(src->path().filename(), line);
    add_breakpoint(pos, [](auto && ...) {}, {src, line});
}


void breakpoint_list::connect_impl(breakpoint_list_impl * impl) {
    impl_ = impl;
    assert(impl_ != nullptr && "impl pointer should not be null");

    // resetting real hit counters to zero for all breakpoints
    for (auto bp : all()) {
        bp->reset_real_curr_hit_count();
    }

    // installing all code breakpoints into implementation
    for (auto bp : code_breakpoints()) {
        install_code_breakpoint_into_impl(bp);
    }
}


void breakpoint_list::disconnect_impl() {

    // we don't need remove breakpoints because they will be removed
    // automatically after closing target

    // resettings implementation numbers for all breakpoints
    for (auto bp : all()) {
        bp->set_impl_num(breakpoint::invalid_num);
    }

    // removing all locations for all code breakpoints
    for (auto bp : code_breakpoints()) {
        bp->remove_all_locations();
        changed()(bp);
    }

    list_changed()();

    impl_ = nullptr;
}


breakpoint_id breakpoint_list::bp_id(const breakpoint * bp) {
    if (auto code_bp = dynamic_cast<const code_breakpoint*>(bp)) {
        return {breakpoint_type::code, bp->num()};
    } else if (auto wp = dynamic_cast<const watchpoint*>(bp)) {
        return {breakpoint_type::watch, bp->num()};
    } else {
        assert(false && "Unknown breakpoint type");
        return {breakpoint_type::code, 0};
    }
}


void breakpoint_list::add_code_breakpoint(const std::shared_ptr<code_breakpoint> & bp,
                                          const code_breakpoint_handler & h,
                                          const source_position & toggle_pos) {

    // creating breakpoint object and adding it into map of breakpoints
    auto [it, was_added] = breakpoints_.emplace(bp_id(bp.get()), bp);
    assert(was_added && "breakpoint with number already exists");

    // notifying users about new breakpoint
    added()(bp.get());
    list_changed()();

    // invoking completion handler
    h(bp.get());

    if (!impl_) {
        return;
    }

    // installing breakpoint into implementation
    install_code_breakpoint_into_impl(bp.get(), toggle_pos);
}


void breakpoint_list::install_code_breakpoint_into_impl(code_breakpoint * bp,
                                                        const source_position & toggle_pos) {
    auto bp_num = bp->num();
    bp->install_into_impl(*impl_,
    [this, bp_num, toggle_pos](const code_breakpoint_info & bp_inf) {
        auto it = breakpoints_.find(breakpoint_id{breakpoint_type::code, bp_num});
        if (it == breakpoints_.end()) {
            // breakpoint was deleted before installation is completed
            return;
        }

        // updating breakpoint implementation number
        assert(it->second->impl_num() == breakpoint::invalid_num &&
               "breakpoint implementation number should not be set");
        it->second->set_impl_num(bp_inf.num());

        // updating breakpoint locations
        auto cbp = dynamic_cast<code_breakpoint*>(it->second.get());
        assert(cbp && "breakpoint is not a code breakpoint");
        add_breakpoint_locations(cbp, bp_inf);

        changed()(cbp);
        list_changed()();


        // if breakpoint was added via toggle, and actual breakpoint location differs
        // from toggle location, then notifying users about it

        auto code_bp = dynamic_cast<const code_breakpoint*>(it->second.get());

        if (toggle_pos) {
            for (auto && loc : code_bp->locations()) {
                if (!loc->pos().src_pos())
                    continue;

                if (loc->pos().src_pos().file() == toggle_pos.file() &&
                    loc->pos().src_pos() != toggle_pos) {

                    toggle_pos_changed()(loc->pos().src_pos());
                }
            }
        }
    });
}


}
