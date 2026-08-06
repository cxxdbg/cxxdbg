// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file settings_storage.hpp
/// Contains definition of the settings_storage class.

#pragma once

#include <list>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>


namespace cxxdbg {


/// \class settings_storage
/// Represents abstract storage for application settings
class settings_storage {
public:
    /// Destructor, destroys object
    virtual ~settings_storage();

    /// Reads value with specified name and default value from storage
    template <typename T> T read(const std::string & name, const T & def) const;

    /// Writes value with specified name to storage
    template <typename T> void write(const std::string & name, const T & val);

private:
    /// Reads string value with specified name from storage. Returns true
    /// if storage contains value.
    virtual bool read_string(const std::string & name, std::string & val) const = 0;

    /// Writes string value with specified name to storage
    virtual void write_string(const std::string & name, const std::string & val) = 0;
};


/// Splits string into vector of strings
bool settings_storage_split_vector(const std::string & str, std::vector<std::string> & v);

/// Converts vector of strings to string
std::string settings_storage_merge_vector(const std::vector<std::string> & v);



/// Settings storage traits
template <typename T>
struct settings_storage_traits {
    /// Converts value to string
    static std::string to_string(const T & val) {
        return boost::lexical_cast<std::string>(val);
    }

    /// Converts string to value
    static bool from_string(const std::string & str, T & val) {
        try {
            val = boost::lexical_cast<T>(str);
            return true;
        }
        catch(const boost::bad_lexical_cast &) {
            return false;
        }
    }
};


/// Settings storage traits for container
template <typename T, typename Cont>
struct settings_storage_traits_container {
    /// Converts value to string
    static std::string to_string(const Cont & vals) {
        std::vector<std::string> str_vals;
        for (auto && v : vals) {
            str_vals.push_back(settings_storage_traits<T>::to_string(v));
        }
        return settings_storage_merge_vector(str_vals);
    }

    /// Converts string to value
    static bool from_string(const std::string & str, Cont & vals) {
        std::vector<std::string> str_vals;
        if (!settings_storage_split_vector(str, str_vals))
            return false;

        vals.clear();
        for (auto && s : str_vals) {
            T v;
            if (!settings_storage_traits<T>::from_string(s, v))
                return false;
            vals.push_back(v);
        }

        return true;
    }
};


/// Settings storage traits for std::vector
template <typename T>
struct settings_storage_traits<std::vector<T>>:
public settings_storage_traits_container<T, std::vector<T>> {};


/// Settings storage traits for std::list
template <typename T>
struct settings_storage_traits<std::list<T>>:
public settings_storage_traits_container<T, std::list<T>> {};


template <typename T>
T settings_storage::read(const std::string & name, const T & def) const {
    std::string val;
    bool ok = read_string(name, val);
    if (!ok)
        return def;

    T tval;
    ok = settings_storage_traits<T>::from_string(val, tval);
    if (!ok)
        return def;

    return tval;
}


template <typename T>
void settings_storage::write(const std::string & name, const T & val) {
    write_string(name, settings_storage_traits<T>::to_string(val));
}


}


