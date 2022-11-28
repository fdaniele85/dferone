//
// Created by daniele on 24/11/22.
//

#pragma once

#include "tmp.h"

namespace dferone::containers {
    /** @typdef count_method_type
     *  @brief  This is the type of a generic .count() method for a container.
     *
     *  This method is implemented in some stl containers and takes an element as its only parameter.
     */
    template<class Container, class T>
    using count_method_type = decltype(std::declval<Container>().count(std::declval<T>()));

    /** @typedef    has_count_method
     *  @brief      This will inherit from true_type iff class Container has an appopriate .count() method.
     */
    template<class Container, class T>
    using has_count_method = tmp::can_apply<count_method_type, Container, T>;

    namespace detail {
        // Overload of contains for when .count() is not available.
        template<class Container, class T>
        inline bool contains(std::false_type, const Container& container, const T& element) {
            return std::find(container.begin(), container.end(), element) != container.end();
        }

        // Overload of contains for when .count() is available.
        template<class Container, class T>
        inline bool contains(std::true_type, const Container& container, const T& element) {
            return container.count(element) > 0u;
        }
    }

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
    template<class Container, class T>
    inline bool contains(const Container& container, const T& element) {
        return detail::contains(has_count_method<const Container&, const T&>{}, container, element);
    };
}