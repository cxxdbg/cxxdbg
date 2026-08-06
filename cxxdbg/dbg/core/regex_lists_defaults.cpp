// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "regex_lists_defaults.hpp"


namespace cxxdbg::dbg::core {

namespace functions_to_skip {

/// creates and returns default value for the names of functions to skip when debugging
regex_lists default_value() {
    regex_list_item item("C++ Standard Library functions", true);

    item.add("^std::");
    item.add("^__gnu_cxx::");

    regex_lists list_of_lists;

    list_of_lists.add(item);

    return list_of_lists;
}

const std::string & default_storage_name() {
    static std::string storage_name = "skip_functions";
    return storage_name;
}

}


namespace functions_to_step_into {
/// creates and returns default value for the names of functions to skip when debugging
regex_lists default_value() {

    regex_lists list_of_lists;

    regex_list_item stdcpp("libstdc++ functional objects", true);
    stdcpp.add("^std::function<.*>::operator\\(\\)");
    stdcpp.add("^std::_Function_base");
    stdcpp.add("^std::_Function_handler");
    stdcpp.add("^std::_Bind");
    stdcpp.add("^std::_Mem_fn_base");
    stdcpp.add("^std::__invoke");
    stdcpp.add("^std::__invoke_impl");
    list_of_lists.add(stdcpp);

    regex_list_item libcxx("libc++ functional objects", true);
    libcxx.add("^std::__[[:alpha:]]*1::function<.*>::operator\\(\\)");
    libcxx.add("^std::__[[:alpha:]]*1::__function::");
    libcxx.add("^std::__[[:alpha:]]*1::__invoke");
    libcxx.add("^std::__[[:alpha:]]*1::__bind");
    libcxx.add("^std::__[[:alpha:]]*1::__apply_functor");
    libcxx.add("^std::__[[:alpha:]]*1::mem_fun");
    list_of_lists.add(libcxx);

    return list_of_lists;
}

const std::string & default_storage_name() {
    static std::string storage_name = "stepinto_functions";
    return storage_name;
}
}

namespace functions_to_group {
/// creates and returns default value for the names of functions to skip when debugging
regex_lists default_value() {

    regex_lists list_of_lists;

    {
        regex_list_item item("libstdc++ functional objects", true);
        item.add("^std::function<.*>::operator\\(\\)");
        item.add("^std::_Function_base");
        item.add("^std::_Function_handler");
        item.add("^std::_Bind");
        item.add("^std::_Mem_fn_base");
        item.add("^std::__invoke");
        item.add("^std::__invoke_impl");

        list_of_lists.add(item);
    }

    {
        regex_list_item item("libc++ functional objects", true);
        item.add("^std::__[[:alpha:]]*1::function<.*>::operator\\(\\)");
        item.add("^std::__[[:alpha:]]*1::__function::");
        item.add("^std::__[[:alpha:]]*1::__invoke");
        item.add("^std::__[[:alpha:]]*1::__bind");
        item.add("^std::__[[:alpha:]]*1::__apply_functor");
        item.add("^std::__[[:alpha:]]*1::mem_fun");

        list_of_lists.add(item);
    }

    return list_of_lists;
}

const std::string & default_storage_name() {
    static std::string storage_name = "group_functions";
    return storage_name;
}
}

}
