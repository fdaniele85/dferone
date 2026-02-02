//
// Created by daniele on 04/12/22.
//

#pragma once

#include <lemon/core.h>
#include <ranges>

namespace dferone::ranges {
    namespace detail {
        class sentinel {};
        template<typename lemon_iterator>
        class iterator {
        public:
            using difference_type = int;
            using value_type = lemon_iterator;
            iterator() : it_(lemon::INVALID) {}
            iterator(lemon_iterator it) : it_(it) {}
            iterator(iterator &&other) : it_(std::move(other.it_)) {}
            lemon_iterator &operator*() const { return it_; }
            lemon_iterator &operator*() { return it_; }
            bool operator==(const iterator &other) const { return it_ == other.it_; }
            bool operator==(const sentinel &s) const { return it_ == lemon::INVALID; }
            iterator &operator=(const iterator &o) {
                it_ = o.it_;
                return *this;
            }
            iterator &operator++() {
                ++it_;
                return *this;
            }

            iterator &operator++(int) {
                auto old = *this;
                ++it_;
                return old;
            }
            lemon_iterator it_;
        };

        template<class lemon_iterator>
        class iterable {
        public:
            explicit iterable(lemon_iterator it) : begin_(it) {}
            iterator<lemon_iterator> begin() { return iterator{begin_}; }

            sentinel end() { return {}; }

        private:
            lemon_iterator begin_;
        };
    } // namespace detail

    template<typename lemon_iterator>
    auto make_lemon_range(lemon_iterator it) {
        return detail::iterable<lemon_iterator>(it);
    }

    template<typename Graph>
    auto forward_star(const Graph &g, const typename Graph::Node &node) {
        return make_lemon_range(typename Graph::OutArcIt(g, node));
    }

    template<typename Graph>
    auto backward_star(const Graph &g, const typename Graph::Node &node) {
        return make_lemon_range(typename Graph::InArcIt(g, node));
    }

    template<typename Graph>
        requires requires { typename Graph::EdgeIt; }
    auto edges(const Graph &g) {
        return make_lemon_range(typename Graph::EdgeIt(g));
    }

    template<typename Graph>
    auto arcs(const Graph &g) {
        return make_lemon_range(typename Graph::ArcIt(g));
    }

    template<typename Graph>
    auto nodes(const Graph &g) {
        return make_lemon_range(typename Graph::NodeIt(g));
    }

} // namespace dferone::ranges
