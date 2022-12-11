//
// Created by daniele on 04/12/22.
//

#pragma once

#include <lemon/core.h>
#include <ranges>

namespace dferone::ranges {
    namespace detail {
        template <class lemon_iterator>
        class iterable {
        public:
            class iterator {
            public:
                using difference_type = int;
                iterator(lemon_iterator it) : it_(it) {}
                auto operator*() {return it_;}
                bool operator==(const iterator &other) const {
                    return it_ == other.it_;
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
            explicit iterable(lemon_iterator it) : begin_(it), end_(lemon::INVALID){
            }
            iterator begin() {
                return iterator{begin_};
            }

            iterator end() {
                return iterator{end_};
            }

        private:
            lemon_iterator begin_;
            lemon::Invalid end_;
        };
    }

    template <typename lemon_iterator>
    auto make_lemon_range(lemon_iterator it) {
        return detail::iterable<lemon_iterator>(it);
    }

    template <typename Graph>
    auto forward_star(const Graph &g, const typename Graph::Node &node) {
        return make_lemon_range(typename Graph::OutArcIt(g, node));
    }

    template <typename Graph>
    auto backward_star(const Graph &g, const typename Graph::Node &node) {
        return make_lemon_range(typename Graph::InArcIt(g, node));
    }

    template <typename Graph>
    requires requires {
        typename Graph::EdgeIt;
    }
    auto edges(const Graph &g) {
        return make_lemon_range(typename Graph::EdgeIt(g));
    }

    template <typename Graph>
    auto arcs(const Graph &g) {
        return make_lemon_range(typename Graph::ArcIt(g));
    }

    template <typename Graph>
    auto nodes(const Graph &g) {
        return make_lemon_range(typename Graph::NodeIt(g));
    }


}
