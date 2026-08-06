// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "regex_lists.hpp"


#include <algorithm>
#include <iterator>
#include <assert.h>
#include <iostream>


namespace cxxdbg::dbg::core {
    
// normal constructor
regex_list_item::regex_list_item(const std::string & name, bool enabled):
name_(name), enabled_(enabled)
{
}


regex_list_item::regex_list_item(const std::string & name, container_type && data, bool enabled):
    name_(name),
    data_(std::move(data)),
    enabled_(enabled)
{
}


// assignment operator
regex_lists & regex_lists::operator=(const regex_lists & other)
{
    data_ = other.data_;
    
    return *this;
}


// move assignment operator
void regex_lists::operator=(regex_lists && other)
{
    data_ = std::move(other.data_);
}


// move constructor
regex_lists::regex_lists(const regex_lists && other):
    data_(std::move(other.data_))
{
}


// copy constructor
regex_lists::regex_lists(const regex_lists & other):
    data_(other.data_)
{
}


std::string regex_list_item::combine_regex() const
{
    std::string result;
    
    for (const std::string & expression: data_)
    {
        if (expression.empty()) {
            std::cerr << "empty regular expression, skipping" << std::endl;
            continue;
        }
        
        if (!result.empty()) {
            result += "|";
        }
        
        result += "(" + expression + ")";
    }
    
    return result;
}


bool regex_list_item::operator==(const regex_list_item & other) const
{
    if (name_ != other.name_) {
        return false;
    }
    
    if (size() != other.size()) {
        return false;
    }
    
    if (enabled_ != other.enabled_) {
        return false;
    }
    
    auto it1 = data_.begin();
    auto end1 = data_.end();
    auto it2 = other.data_.begin();
    
    for (;it1 != end1; ++it1, ++it2) {
        if (*it1 != *it2) {
            return false;
        }
    }
    
    return true;
}


regex_list_item regex_list_item::get_empty_item(const std::string & name)
{
    return regex_list_item(name, true);
}


regex_list_item::size_type regex_list_item::size() const
{ 
    return data_.size(); 
}


regex_list_item::item_type& regex_list_item::operator[](int i)
{
    assert (i >= 0 && i < size()); // make sure that index in boundaries
    
    return data_[i];
}


void regex_list_item::remove(int i)
{
    assert (i >= 0 && i < size()); // make sure that index in boundaries
    
    data_.erase(std::begin(data_) + i);
}


bool regex_list_item::add(const item_type & item)
{
    if (std::find(std::begin(data_), std::end(data_), item) == std::end(data_)) {
        data_.push_back(item);
        return true;
    }
    
    return false;
}


void regex_list_item::set_data(const container_type & data)
{
    data_ = data;
}


void regex_list_item::set_name(const std::string & name)
{
    name_ = name;
}


bool regex_lists::has_name(const std::string & name, int skip_index) const
{
    auto it = data_.cbegin();
    auto end = data_.cend();
    
    for (int i = 0; it != end; ++it, ++i) {
        if (i != skip_index) {
            if (it->name() == name) {
                return true;
            }
        }
    }
    
    return false;
}


bool regex_lists::operator==(const regex_lists & other) const
{
    if (size() != other.size()) {
        return false;
    }
    
    // comparing storage names makes no sense
//    if (storage_name_ != other.storage_name_)
//        return false;
    
    auto it1 = data_.cbegin();
    auto end1 = data_.cend();
    auto it2 = other.data_.cbegin();
    
    for (;it1 != end1; ++it1, ++it2) {
        if (!(*it1 == *it2)) {
            return false;
        }
    }
    
    return true;
}


std::string regex_lists::combine_regex() const
{
    std::string result;
    
    for (const item_type & item : data_) {
        if (item.enabled()) {
            std::string regex = item.combine_regex();
            
            if (regex.empty()) {
#ifdef _DEBUG
                std::cerr << "empty regex detected in " << __func__ <<
                        ", skipping" << std::endl;
#endif
                continue;
            }
            
            if (!result.empty()) {
                result += "|";
            }
            
            result += regex;
        }
    }
    
    return result;
}


bool regex_lists::add(const item_type & item)
{
    for (item_type & it : data_) {
        if (it.name() == item.name()) {
            return false;
        }
    }
    
    data_.push_back(item);
    
    return true;
}


void regex_lists::remove(const std::string & name)
{
    auto res = std::remove_if(std::begin(data_), std::end(data_), [name](regex_list_item& item) { 
        return item.name() == name;
    });

    if (res != std::end(data_)) {
        data_.erase(res, std::end(data_));
    }
}

void regex_lists::remove(size_t index)
{
    assert(index >= 0 && index < size());
    
    data_.erase(std::begin(data_) + index);
}


regex_list_item & regex_lists::operator[](int i)
{
    assert(i >= 0 && i < size());
    
    return data_[i];
}


const regex_list_item & regex_lists::operator[](int i) const
{
    assert(i >= 0 && i < size());
    
    return data_[i];
}



size_t regex_lists::size() const
{
    return data_.size(); 
}


}

