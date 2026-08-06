// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file join_delim.hpp
/// Contains definition of the join_delim range adaptor and related classes.

#pragma once


#include <iterator>
#include <type_traits>
namespace xranges {


/// View that concatenates multiple views in single view
template <typename Source, typename T>
#if __cplusplus > 201703L
requires std::ranges::input_range<Source> &&
         (std::ranges::view<std::ranges::range_reference_t<Source>> ||
          std::is_reference_v<std::ranges::range_reference_t<Source>>)
#endif
class join_delim_view: public std::ranges::view_base {
public:
    /// Iterator over range
    template <bool Const>
    class iterator_t {
        /// Type of source range
        using source_t = std::conditional_t<Const, const Source, Source>;

        /// Type of iterator over source range of ranges
        using range_iterator_t = std::ranges::iterator_t<source_t>;

        /// Type of iterator over each range
        using element_iterator_t = std::ranges::iterator_t<std::ranges::range_reference_t<source_t>>;

        /// Empty utility struct
        struct empty {};

        /// True if range reference is real reference
        static constexpr auto range_is_ref = std::is_reference_v<std::ranges::range_reference_t<source_t>>;

        /// Type of cache of current range used in case if source range returns values
        /// not by reference
        using current_range_t = std::conditional_t<
            range_is_ref,
            empty,
            std::ranges::views::all_t<std::ranges::range_reference_t<source_t>>
        >;

    public:
        /// Iterator category
        using iterator_category = std::forward_iterator_tag;

        /// Iterator value type
        using value_type = std::ranges::range_value_t<std::ranges::range_reference_t<source_t>>;

        /// Iterator difference type
        using difference_type = ptrdiff_t;

        /// Iterator reference type
        using reference = T;

        /// Pointer type
        using pointer = T *;

        /// Default constructor
        iterator_t() = default;

        /// Constructs iterator with specified reference to source range and delimiter
        explicit iterator_t(source_t * src, T d):
        src_{src}, delim_{d}, r_it_{std::ranges::begin(*src_)} {
            auto x = std::ranges::begin(*src_);
            set_range_begin();
        }

        /// Constructs iterator pointing to the end of sequence
        explicit iterator_t(source_t * src, T d, bool):
        src_{src}, delim_{d}, r_it_{std::ranges::end(*src_)} {
        }

        /// Returns reference to current element
        reference operator*() const {
            // first checking for delimeneter flag
            if (is_delim_) {
                return delim_;
            }

            // returning current element
            return *it_;
        }

        /// Moves iterator to next element
        auto & operator++() {
            if (is_delim_) {
                // r_it_ / r_ already points to the next range here. We need
                // just set current element iterator to the beginning of new range
                is_delim_ = false;
                set_range_begin();
                return *this;
            }

            // moving to the next element and checking for the end of current range
            ++it_;
            check_for_curr_range_end();

            return *this;
        }

        /// Moves iterator to next element. Returns iterator pointing to previous element
        auto operator++(int) {
            auto curr = *this;
            ++(*this);
            return curr;
        }

        /// Compares this iterator with other
        bool operator==(const iterator_t<Const> & other) const {
            if (src_ != other.src_ || delim_ != other.delim_) {
                return false;
            }

            if (r_it_ != other.r_it_) {
                return false;
            }

            // if range iterator points to the end then iterators are equal
            if (r_it_ == std::ranges::end(*src_)) {
                return true;
            }

            // need compare element iterators
            return it_ == other.it_;
        }

        /// Compares this iterator with other
        bool operator!=(const iterator_t<Const> & other) const {
            return !(*this == other);
        }

    private:
        /// Sets current element iterator to beginning of current range.
        void set_range_begin() {
            if (r_it_ == std::ranges::end(*src_)) {
                // end of sequence reached
                return;
            }

            if constexpr (range_is_ref) {
                it_ = std::ranges::begin(*r_it_);
            } else {
                r_ = *r_it_;
                it_ = std::ranges::begin(r_);
            }

            check_for_curr_range_end();
        }

        /// Checks for end of sequence and set range iterator to the end of source range
        /// if end of sequence is reached
        void check_for_curr_range_end() {
            // checking for the end of current range
            if constexpr (range_is_ref) {
                if (it_ != std::ranges::end(*r_it_)) {
                    return;
                }
            } else {
                if (it_ != std::ranges::end(r_)) {
                    return;
                }
            }

            // moving to the next range
            ++r_it_;

            // checking for end of sequence    
            if (r_it_ == std::ranges::end(*src_)) {
                return;
            }

            // initializing r_ with new value for non-reference ranges
            if constexpr (!range_is_ref) {
                r_ = std::ranges::begin(*r_it_);
            }

            // setting delimeter flag
            is_delim_ = true;
        }

        source_t * src_;            ///< Pointer to source view
        T delim_;                   ///< Delimeter
        range_iterator_t r_it_;     ///< Iterator pointing to current range
        current_range_t r_;         ///< Current range (used only if range returns not references)
        element_iterator_t it_;     ///< Iterator pointing to current element

        /// Delimeter flag. Describes current state of iterator:
        /// 1. If flag is false then r_it_ / r_ points to current range, and it_ points to current
        ///    element in that range.
        /// 2. If flag is true then r_it_ / r_ points to the next range, and it_ is invalid.
        bool is_delim_ = false;
    };

    using iterator = iterator_t<false>;
    using const_iterator = iterator_t<true>;

    /// Constructs empty view
    join_delim_view() = default;

    /// Constructs view with source range and delimeter
    explicit join_delim_view(Source src, T delim):
        src_{std::move(src)}, delim_{std::move(delim)} {}
    
    /// Copy constructor
    join_delim_view(const join_delim_view &) = default;

    /// Move constructor
    join_delim_view(join_delim_view &&) = default;

    /// Copy assignment operator
    join_delim_view & operator=(const join_delim_view &) = default;

    /// Move assignment operator
    join_delim_view & operator=(join_delim_view &&) = default;

    /// Returns iterator pointing to the first element in range
    auto begin() {
        return iterator{&src_, delim_};
    }

    /// Returns iterator pointing to one past end element in range
    auto end() {
        return iterator{&src_, delim_, true};
    }


    /// Returns iterator pointing to the first element in range
    auto begin() const {
        return const_iterator{&src_, delim_};
    }

    /// Returns iterator pointing to one past end element in range
    auto end() const {
        return const_iterator{&src_, delim_, true};
    }


private:
    Source src_;            ///< Source view
    T delim_;               ///< Delimiter
};


/// join_delim closure to be appended with operator | to another view
template <typename T>
struct join_delim_closure {
    /// Constructs closure with specified delimiter
    join_delim_closure(T d): delim{d} {}

    T delim;            ///< Delimiter
};


struct join_delim_fn {
    /// Constructs join_delim view
    template <typename Source, typename T>
#if __cplusplus > 201703L
    requires std::ranges::input_range<Source>
#endif
    constexpr auto operator()(Source && src, T && delim) const {
        return join_delim_view{(std::forward<Source>(src) | std::ranges::views::all), delim};
    }

    /// Constructs join_delim closure with specified delimeter
    template <typename T>
    constexpr auto operator()(T && delim) const {
        return join_delim_closure<std::decay_t<T>>{delim};
    }
};


/// Constructs join_delim view from source view and closure
template <typename Range, typename T>
#if __cplusplus > 201703L
requires (std::ranges::input_range<Range>)
#endif
constexpr auto operator|(Range && r, const join_delim_closure<T> & c) {
    return join_delim_view{r | std::ranges::views::all, c.delim};
}


namespace views {
    constexpr join_delim_fn join_delim = join_delim_fn{};
}


}
