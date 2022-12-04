//
// Created by daniele on 04/12/22.
//

#pragma once

#include <lemon/core.h>

namespace dferone::ranges {
    namespace detail {
        template <class lemon_iterator, class... Types>
        class iterable {
        public:
            class iterator {
            public:
                iterator(lemon_iterator it) : it_(it) {}
                auto operator*() {return it_;}
                bool operator==(const iterator &other) const {
                    return it_ == other.it_;
                }
                iterator operator++() {
                    ++it_;
                    return *this;
                }

                iterator operator++(int) {
                    auto old = *this;
                    ++it_;
                    return old;
                }
            private:
                lemon_iterator it_;
            };
            explicit iterable(Types... args) : begin_(std::forward<Types>(args)...), end_(lemon::INVALID) {}
            iterator begin() {
                return iterator(begin_);
            }

            iterator end() {
                return iterator(end_);
            }

        private:
            lemon_iterator begin_;
            lemon::Invalid end_;
        };
    }

    template <typename T, class... Types>
    detail::iterable<T, Types...> make_lemon_range(Types... args) {
        return detail::iterable<T, Types...>(std::forward<Types>(args)...);
    }


}
