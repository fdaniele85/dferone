//
// Created by daniele on 27/11/22.
//

#pragma once

#include <ranges>
#include <iostream>

namespace dferone::streams {

    template<class Container>
    requires std::ranges::range<Container>
    inline std::ostream &join_and_print(std::ostream &out, const Container &c, const std::string &separator = ", ") {
        bool first = true;
        for (const auto &x: c) {
            if (!first) {
                out << separator;
            }
            out << x;
            first = false;
        }
        return out;
    }

}