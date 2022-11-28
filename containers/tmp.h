//
// Created by Alberto Santini on 11/04/18. (see https://github.com/alberto-santini/as)
//

#ifndef DFERONE_TMP_H
#define DFERONE_TMP_H

#include <type_traits>

/** @namespace  tmp
 *  @brief      Template-meta-programming namespace.
 */
namespace tmp {
    /*
     * The following is TMP magic used to see if some template is a valid expression. See:
     * https://stackoverflow.com/questions/29521010/enable-template-function-if-class-has-specific-member-function
     */
    template<class...>
    struct types {
        using type = types;
    };

    namespace details {
        template<template<class...> class Z, class types, class=void>
        struct can_apply : std::false_type {};

        template<template<class...> class Z, class...Ts>
        struct can_apply<Z, types<Ts...>, std::void_t<Z<Ts...>>> : std::true_type {};
    }

    /** @typedef can_apply
     *  @brief   Checks if the application of arguments to a templated class is valid.
     *
     *  can_apply<my_template, my_arguments...> will inherit from true_type iff
     *  my_template<my_arguments...> is a valid expression.
     *
     *  @tparam Z   The templated class.
     *  @tparam Ts  The template classes of \p Z.
     */
    template<template<class...> class Z, class...Ts>
    using can_apply = details::can_apply<Z, types<Ts...>>;
}

#endif //DFERONE_TMP_H
