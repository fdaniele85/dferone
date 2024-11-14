//
// Created by daniele on 24/11/22.
//

#pragma once

#include <sstream>
#include <tuple>
#include <iostream>

namespace dferone::containers {
    /** @brief  Tells whether a container contains a certain element.
     *
     *  The standard library's functions to find elements (e.g. std::find)
     *  always return an iterator. Sometimes, though, we just want to know
     *  whether an element is in a container or not. This helper function
     *  lets us do this in a concise way. This function also has specialisation
     *  for when the container implements a .count() method, i.e. a more
     *  efficient way of searching elements than simple linear search.
     *
     *  @tparam Container    Container type.
     *  @tparam T            Containee type.
     *  @param  container    The container.
     *  @param  element      The element we are searching in \p container.
     *  @return              True iff \p element was found in \p container.
     */
    template<std::ranges::range Container, class T>
    inline bool contains(const Container& container, const T& element) {
        return std::find(container.begin(), container.end(), element) != container.end();
    };

    template<std::ranges::range Container, class T>
    inline bool contains(const Container& container, const T& element)
    requires requires {
        {container.count(element)} -> std::convertible_to<typename Container::size_type>;
    }
    {
        return container.count(element) > 0u;
    };

    namespace detail {
        // Prints a value to an output stream.
        template<class Value>
        inline std::ostream& iterated_value(const Value& value, std::ostream& out) {
            return out << value;
        }

        // Prints a key-value pair to an output stream.
        template<class Key, class Value>
        inline std::ostream& iterated_value(const std::pair<Key, Value>& key_val, std::ostream& out) {
            return out << key_val.first << ": " << key_val.second;
        }
    }

    /** @brief  Joins the elements of a container using the separator, and prints the result to
     *          an output stream.
     *
     *  It works both with key-value and value-only containers.
     *  In the first case, it prints both key and value.
     *
     *  The code below is not mine, but by Alberto Santini and it was
     *  origianlly available at https://github.com/alberto-santini/as
     *
     *  @tparam Container    Container type.
     *  @param  container    The container to print.
     *  @param  out          Output stream.
     *  @param  separator    A string interposed between two adjacent elements.
     */
    template<class Container>
    inline void join_and_print(const Container& container, std::ostream& out = std::cout, std::string separator = ", ") {
        if(container.begin() == container.end()) { return; }
        for(auto it = container.begin(); it != std::prev(container.end()); ++it) {
            detail::iterated_value(*it, out) << separator;
        }
        auto last = std::prev(container.end());
        detail::iterated_value(*last, out);
    }

    /** @brief  Joins the elements of a container using the separator, and prints the result to
     *          the standard output stream.
     *
     *  It works both with key-value and value-only containers.
     *  In the first case, it prints both key and value.
     *
     *  The code below is not mine, but by Alberto Santini and it was
     *  origianlly available at https://github.com/alberto-santini/as
     *
     *  @tparam Container    Container type.
     *  @param  container    The container to print.
     *  @param  separator    A string interposed between two adjacent elements.
     */
    template<typename Container>
    inline void join_and_print(const Container& container, std::string separator) {
        join_and_print(container, std::cout, separator);
    }

    template<std::ranges::range Container>
    inline std::string to_string(const Container &c, const std::string &separator = ", ") {
        std::ostringstream ss;
        join_and_print(c, ss, separator);
        return ss.str();
    }

    /** @brief  Iterates over an iterable container and yields both the index and
    *          the element.
    *
    *          This method works analogously to Python's enumerate().
    *          The code below is not mine, but by Nathan Reed and it was
    *          origianlly available at http://reedbeta.com/blog/python-like-enumerate-in-cpp17/
    *
    *  @tparam Container   The container type.
    *  @param  iterable    An instance of the iterable container.
    *  @return             An anonymous struct implementing begin() and end(). When passed in
    *                      a range-based for loop, each element gives a tuple whose second element
    *                      is an iterable element, and whose first element is the corresponding
    *                      index.
    */
    template<   typename Container,
            typename Iter = decltype(std::begin(std::declval<Container>())),
            typename = decltype(std::end(std::declval<Container>()))>
    constexpr auto enumerate(Container && iterable) {
        struct iterator {
            std::size_t i;
            Iter iter;

            bool operator!=(const iterator& other) const { return iter != other.iter; }
            void operator++() { ++i; ++iter; }
            auto operator*() const { return std::tie(i, *iter); }
        };

        struct iterable_wrapper {
            Container iterable;

            auto begin() { return iterator{ 0u, std::begin(iterable) }; }
            auto end() { return iterator{ 0u, std::end(iterable) }; }
        };

        return iterable_wrapper{ std::forward<Container>(iterable) };
    }

}