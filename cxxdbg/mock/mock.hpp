// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file mock.hpp
/// Contains definitions of mock framework classes and macroses.

#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <vector>


namespace cxxdbg { namespace mock {


template <typename T>
struct function;

template <typename Res>
struct function<Res()> {
    typedef Res result_type;
};

template <typename Res, typename T1>
struct function<Res (T1)>: public function<Res()> {
    typedef T1 arg1_type;
};

template <typename Res, typename T1, typename T2>
struct function<Res (T1, T2)>: public function<Res(T1)> {
    typedef T2 arg2_type;
};

template <typename Res, typename T1, typename T2, typename T3>
struct function<Res (T1, T2, T3)>: public function<Res(T1, T2)> {
    typedef T3 arg3_type;
};

template <typename Res, typename T1, typename T2, typename T3, typename T4>
struct function<Res (T1, T2, T3, T4)>: public function<Res(T1, T2, T3)> {
    typedef T4 arg4_type;
};

template <typename Res, typename T1, typename T2, typename T3, typename T4, typename T5>
struct function<Res (T1, T2, T3, T4, T5)>: public function<Res(T1, T2, T3, T4)> {
    typedef T5 arg5_type;
};

template <typename Res, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct function<Res (T1, T2, T3, T4, T5, T6)>: public function<Res(T1, T2, T3, T4, T5)> {
    typedef T6 arg6_type;
};

template <typename Res, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct function<Res (T1, T2, T3, T4, T5, T6, T7)>: public function<Res(T1, T2, T3, T4, T5, T6)> {
    typedef T7 arg7_type;
};

template <typename Res, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct function<Res (T1, T2, T3, T4, T5, T6, T7, T8)>: public function<Res(T1, T2, T3, T4, T5, T6, T7)> {
    typedef T8 arg8_type;
};


template <typename Ret, typename ... Args>
class method_call {
public:
    method_call(const method_call &) = default;

    template <typename Func>
    explicit method_call(const std::string & f,
                         unsigned int l,
                         unsigned int cn,
                         const Func & action):
        file_{f}, line_{l}, call_num_{cn}, action_{action} {}

    Ret do_call(unsigned int n, Args ... args) const {
        if (n != call_num_) {
            std::ostringstream msg;
            msg << "mock: unexpected call registered at " << file_ << ":" << line_;
            throw std::runtime_error(msg.str());
        }

        return action_(args...);
    }

private:
    std::string file_;
    unsigned int line_;
    unsigned int call_num_;
    std::function<Ret (Args...)> action_;
};



template <typename Method>
struct make_method_call;

template <typename Res, typename Cls, typename ... Args>
struct make_method_call<Res (Cls::*)(Args...)> {
    typedef method_call<Res, Args...> type;
};

template <typename Res, typename Cls, typename ... Args>
struct make_method_call<Res (Cls::*)(Args...) const> {
    typedef method_call<Res, Args...> type;
};



template <typename Res, typename ... Args>
class method {
public:
    template <typename Func>
    void add_call(const std::string & file,
                  unsigned int line,
                  unsigned int n,
                  const Func & action) {
        calls_.push_back(method_call<Res, Args...>{file, line, n, action});
    }

    Res do_call(unsigned int n, Args ... args) const {
        if (curr_call_num_ >= calls_.size()) {
            std::ostringstream msg;
            msg << "mock: unexpected call number " << n;
            throw std::runtime_error(msg.str());
        }

        return calls_[curr_call_num_++].do_call(n, args...);
    }

private:
    std::vector<method_call<Res, Args...>> calls_;
    mutable std::size_t curr_call_num_ = 0;
};



template <typename Method>
struct make_method;

template <typename Res, typename Cls, typename ... Args>
struct make_method<Res (Cls::*)(Args...)> {
    typedef method<Res, Args...> type;
};

template <typename Res, typename Cls, typename ... Args>
struct make_method<Res (Cls::*)(Args...) const> {
    typedef method<Res, Args...> type;
};

template <typename Res, typename ... Args>
struct make_method<Res (Args...)> {
    typedef method<Res, Args...> type;
};


/// Base class of all mock objects
class object_base {
public:
    unsigned int inc_curr_reg_call_num() { return curr_reg_call_num_++; }
    unsigned int inc_curr_call_num() const { return curr_call_num_++; }

    bool verify() const { return curr_reg_call_num_ == curr_call_num_; }

private:
    unsigned int curr_reg_call_num_ = 0;
    mutable unsigned int curr_call_num_ = 0;
};


/// Mock objects. Should be base class for all mock objects
template <typename Child>
struct object: virtual public object_base {
    typedef Child mock_type;
};


#define MOCK_DEFINE_METHOD_0_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name() const override { \
        return name##_mock.do_call(inc_curr_call_num()); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_0(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name() override { \
        return name##_mock.do_call(inc_curr_call_num()); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_1(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_1_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_2(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_2_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_3(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_3_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;

#define MOCK_DEFINE_METHOD_4(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_5(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_5_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_6(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5, \
                                                    ::cxxdbg::mock::function<mtype>::arg6_type arg6) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5, arg6); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_6_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5, \
                                                    ::cxxdbg::mock::function<mtype>::arg6_type arg6) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5, arg6); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_7(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5, \
                                                    ::cxxdbg::mock::function<mtype>::arg6_type arg6, \
                                                    ::cxxdbg::mock::function<mtype>::arg7_type arg7) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5, arg6, arg7); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_7_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5, \
                                                    ::cxxdbg::mock::function<mtype>::arg6_type arg6, \
                                                    ::cxxdbg::mock::function<mtype>::arg7_type arg7) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5, arg6, arg7); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_8(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5, \
                                                    ::cxxdbg::mock::function<mtype>::arg6_type arg6, \
                                                    ::cxxdbg::mock::function<mtype>::arg7_type arg7, \
                                                    ::cxxdbg::mock::function<mtype>::arg8_type arg8) override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_DEFINE_METHOD_8_C(name, mtype) \
    ::cxxdbg::mock::function<mtype>::result_type name(::cxxdbg::mock::function<mtype>::arg1_type arg1, \
                                                    ::cxxdbg::mock::function<mtype>::arg2_type arg2, \
                                                    ::cxxdbg::mock::function<mtype>::arg3_type arg3, \
                                                    ::cxxdbg::mock::function<mtype>::arg4_type arg4, \
                                                    ::cxxdbg::mock::function<mtype>::arg5_type arg5, \
                                                    ::cxxdbg::mock::function<mtype>::arg6_type arg6, \
                                                    ::cxxdbg::mock::function<mtype>::arg7_type arg7, \
                                                    ::cxxdbg::mock::function<mtype>::arg8_type arg8) const override { \
        return name##_mock.do_call(inc_curr_call_num(), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); \
    } \
    \
    ::cxxdbg::mock::make_method<decltype(&mock_type::name)>::type name##_mock;


#define MOCK_ADD_CALL(obj, name, action) \
    (obj).name##_mock.add_call(__FILE__, __LINE__, (obj).inc_curr_reg_call_num(), action)



} }


