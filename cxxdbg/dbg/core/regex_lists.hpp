// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/* 
 * File:   regex_lists.hpp
 * Author: extremer
 *
 * Created on 25 Июль 2016 г., 8:42
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace cxxdbg::dbg::core {

/// list of regular expressions
class regex_list_item
{
public:
    typedef std::string item_type;
    typedef std::vector<item_type> container_type;
    typedef typename container_type::size_type size_type;
    
    /// normal constructor
    regex_list_item(const std::string & name, bool enabled = false);
    
    /// partial copy constructor
    regex_list_item(const std::string & name, container_type && data, bool enabled = false);
    
    /// returns list name
    const std::string & name() const { return name_; }
    
    /// returns list of regular expressions
    const container_type & data() const { return data_; }
    
    /// returns whether list is enabled
    bool enabled() const { return enabled_; }
    
    /// enables or disables item
    void enable(bool flag) { enabled_ = flag; }
    
    /// adds new list
    bool add(const item_type & item);
    
    /// sets data
    void set_data(const container_type & data);
    
    /// sets name
    void set_name(const std::string & name);
    
    /// returns combined regular expression
    std::string combine_regex() const;
    
    /// compares two objects
    bool operator==(const regex_list_item & other) const;
    
    /// creates and returns empty regex list item with specified name
    static regex_list_item get_empty_item(const std::string & name);
    
    /// returns count of items in container
    size_type size() const;
    
    /// returns link to i-th item
    item_type& operator[](int i);
    
    /// removes item by index
    void remove(int i);
    
private:
    std::string name_;      ///< list name
    container_type data_;   ///< list of regex
    bool enabled_;          ///< flag
};


class regex_lists
{
public:
    typedef regex_list_item item_type;
    typedef std::vector<regex_list_item> container_type;
    typedef typename container_type::size_type size_type;
    
    /// normal constructor
    regex_lists() = default;
    
    /// normal operator=
    regex_lists& operator=(const regex_lists & other);
    
    /// move operator= for non-const only
    void operator=(regex_lists && other);
    
    /// move constructor
    regex_lists(const regex_lists && other);
    
    /// copy constructor
    regex_lists(const regex_lists & other);
    
    /// creates combined regex, where only enabled items are included
    std::string combine_regex() const;
    
    /// adds item if its name is not in the list yet
    bool add(const item_type & item);
    
    /// removes list item with specified name
    void remove(const std::string & name);
    
    /// removes list item by index
    void remove(size_t index);
    
    /// creates and returns default value
    //static regex_lists default_value(const std::string & storage_name);
    
    /// returns reference to i-th item
    regex_list_item & operator[](int i);

    /// returns const reference to i-th item
    const regex_list_item & operator[](int i) const;
    
    /// returns count of items
    size_t size() const;
    
    /// check if list name exists
    bool has_name(const std::string & name, int skip_index = -1 /*non-existant index*/) const;
    
    /// equility operator
    bool operator==(const regex_lists & other) const;

    /// Non-equality operator
    bool operator!=(const regex_lists & other) const { return !(*this == other); }
    
    /// begin iterator specially for "for" cycles
    auto begin() { return data_.begin(); }
    
    /// end iterator specially for "for" cycles
    auto end() { return data_.end(); }

    /// begin iterator specially for "for" cycles
    auto begin() const { return data_.begin(); }

    /// end iterator specially for "for" cycles
    auto end() const { return data_.end(); }

private:
    container_type data_;   ///< data
};

}
