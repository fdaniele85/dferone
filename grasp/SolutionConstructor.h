//
// Created by Daniele Ferone on 19/05/23.
//

#pragma once

#include <random>
#include <memory>

namespace dferone::grasp {

    template<class ProblemInstance, class Solution>
    struct SolutionConstructor {
        virtual Solution createSolution(const ProblemInstance& instance, std::mt19937& mt) = 0;
        virtual std::unique_ptr<SolutionConstructor<ProblemInstance, Solution>> clone() const = 0;
    };
}