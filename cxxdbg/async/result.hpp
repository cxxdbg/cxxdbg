// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file result.hpp
/// Contains definition of the result class and related classes.

#pragma once

#include "forward.hpp"
#include <cassert>
#include <string>


namespace cxxdbg { namespace async {


/// \class result_base
/// Base class for all asynchronous results specializations
class result_base {
public:
    /// Makes result with ok status
    result_base();

    /// Makes result with error status
    result_base(const std::string & err);

    /// Returns result status
    bool is_ok() const;

    /// Returns result error
    const std::string & error() const;

    /// Changes result status to error
    void set_error(const std::string & err);

protected:
    /// Changes result status to ok
    void set_ok();

private:
    bool is_ok_;        ///< Result status
    std::string err_;   ///< Result error
};


/// \class result
/// Result of exection of asynchronous command
template <typename Type>
class result: public result_base {
public:
    static_assert(std::is_default_constructible<Type>::value,
                  "Type in async result must be default constructible");

    /// Makes result with ok status
    result(const Type & v): value_{v} {}

    /// Makes result with error status
    result(const std::string & err): result_base(err) {}

    /// Returns result value
    const Type & value() const {
        assert(is_ok() && "Invalid result status");
        return value_;
    }

    /// Changes result stateus to ok with specified value
    void set_value(const Type & val) {
        set_ok();
        value_ = val;
    }

    /// Copy constructor from any result type
    template <typename T>
    result(const result<T> & r): result_base{r} {
        if (is_ok())
            set_value(static_cast<Type>(r.value()));
    }

    /// Assignment operator from any result type
    template <typename T>
    result & operator=(const result<T> & r) {
        if (r.is_ok())
            set_value(static_cast<Type>(r.value()));
        else
            set_error(r.error());

        return *this;
    }

    /// Conversion to result with another type
    template <typename T>
    result<T> to() const {
        return result<T>(*this);
    }

private:
    Type value_;        ///< Result value
};


/// \class result
/// std::string specialization for result class
template <>
class result<std::string>: public result_base {
public:
    /// Makes result with error status
    result(const std::string & error): result_base{error} {}

    /// Returns result value
    const std::string & value() const {
        assert(is_ok() && "Invalid result status");
        return value_;
    }

    /// Changes result stateus to ok with specified value
    void set_value(const std::string & val) {
        set_ok();
        value_ = val;
    }

    /// Copy constructor from any result type
    template <typename T>
    result(const result<T> & r): result_base{r} {
        if (is_ok())
            set_value(static_cast<std::string>(r.value()));
    }

    /// Assignment operator from any result type
    template <typename T>
    result & operator=(const result<T> & r) {
        if (r.is_ok())
            set_value(static_cast<std::string>(r.value()));
        else
            set_error(r.error());

        return *this;
    }

    /// Conversion to result with another type
    template <typename T>
    result<T> to() const {
        return result<T>(*this);
    }


private:
    std::string value_;        ///< Result value
};


/// void specialization of result class
template <>
class result<void>: public result_base {
public:
    /// Makes result with ok status
    result() {}

    /// Makes result with error status
    result(const std::string & err): result_base(err) {}

    /// Copy constructor from any async result
    template <typename T>
    result(const result<T> & r):
        result_base{r} {}

    /// Assignment operator from any async result
    template <typename T>
    result & operator=(const result<T> & r) {
        result_base::operator=(r);
        return *this;
    }

    /// Changes result status to ok
    void set_ok() {
        result_base::set_ok();
    }

    /// Converts void result typed result with specified value
    template <typename T>
    result<T> to(const T & v) const {
        if (is_ok())
            return result<T>{v};
        else
            return result<T>{error()};
    }
};


/// Creates ok result with specified value
template <typename T>
result<T> ok_result(const T & val) {
    return result<T>{val};
}

/// Async result handler
template <typename T>
using result_handler = std::function<void (const result<T> & res)>;


} }


