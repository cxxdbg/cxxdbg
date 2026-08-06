// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file stack_frame_compare.hpp
/// Contains definition of comparator class for core::stack_frame objects

#pragma once

#include "cxxdbg/dbg/core/stack_frame.hpp"


namespace cxxdbg::dbg::dapsrv {


/// Stack frame compare functor
struct stack_frame_compare {
    bool operator()(const core::stack_frame & frm1, const core::stack_frame & frm2) const {
        if (frm1.read_pos().load_addr() < frm2.read_pos().load_addr()) {
            return true;
        } else if (frm1.read_pos().load_addr() > frm2.read_pos().load_addr()) {
            return false;
        }

        if (frm1.read_pos().file_addr() < frm2.read_pos().file_addr()) {
            return true;
        } else if (frm1.read_pos().file_addr() > frm2.read_pos().file_addr()) {
            return false;
        }

        if (frm1.get_cfa() < frm2.get_cfa()) {
            return true;
        } else if (frm1.get_cfa() > frm2.get_cfa()) {
            return false;
        }

        if (frm1.get_sc_id() < frm2.get_sc_id()) {
            return true;
        } else if (frm1.get_sc_id() > frm2.get_sc_id()) {
            return false;
        }

        return false;
    }
};


}
