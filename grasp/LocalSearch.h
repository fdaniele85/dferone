//
// Created by Daniele Ferone on 19/05/23.
//

#pragma once

#include <random>
#include <memory>

namespace dferone::grasp {

    template<class Solution>
    struct LocalSearch {
        virtual void search(Solution &s, std::mt19937& mt) = 0;
        virtual std::unique_ptr<LocalSearch<Solution>> clone() const = 0;
    };
}