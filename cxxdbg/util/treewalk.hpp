// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <functional>

namespace cxxdbg::dbg::util::tree {

/// class TreeWalker is used for visiting all nodes of trees
template <class T>
class tree_walker {
public:
    typedef std::function<void(const T&, size_t shift)> visit_func;
    typedef std::function<size_t(const T&)> count_func;
    typedef std::function<const T&(const T&, size_t)> at_func;

    tree_walker(visit_func visit, count_func count, at_func at): visit_(visit), count_(count), at_(at) { }

    void operator()(const T & node, int shift = 0) {
        visit_(node, shift);
        for (size_t i = 0; i < count_(node); ++i) {
            auto child = at_(node, i);
            (*this)(child, shift + 2);
        }
    }

private:
    visit_func visit_;
    count_func count_;
    at_func at_;
};

namespace detail {
template <class T, class Visit, class Count, class At>
class tree_walker1 {
public:
    tree_walker1(Visit v, Count c, At a): visit_(v), count_(c), at_(a) { }

    void operator()(const T node, int shift = 0) {
        visit_(node, shift);
        for (size_t i = 0; i < count_(node); ++i) {
            auto child = at_(node, i);
            (*this)(child, shift + 2);
        }
    }

private:
    Visit visit_;
    Count count_;
    At at_;
};
}

template <class T>
struct make {
    template<class V, class C, class A>
    static auto tree_walker1(V v, C c, A a) {
        return detail::tree_walker1<T, V, C, A>(v, c, a);
    }
};


template <class L, class R>
struct tree_comparator {
    using compare_func = std::function<bool(const L &, const R &)>;
    using left_count_func = std::function<size_t (const L &)>;
    using left_get_func = std::function<const L& (const L&, size_t)>;
    using right_count_func = std::function<size_t (const R &)>;
    using right_get_func = std::function<const R& (const R &, size_t)>;

    tree_comparator(compare_func compare,
                    left_count_func lc, left_get_func lg,
                    right_count_func rc, right_get_func rg):
        compare_{compare},
        lc_{lc}, lg_{lg},
        rc_{rc}, rg_{rg} {
    }

    bool operator()(const L & l, const R & r) {
        if (! compare_(l, r))
            return false;
        size_t count = lc_(l);
        if (count != rc_(r))
            return false;

        for (size_t i = 0; i < count; ++i) {
            const auto & lchild = lg_(l, i);
            const auto & rchild = rg_(r, i);
            if (! (*this)(lchild, rchild))
                return false;
        }

        return true;
    }

private:
    compare_func compare_;
    left_count_func lc_;
    left_get_func lg_;
    right_count_func rc_;
    right_get_func rg_;
};

}

