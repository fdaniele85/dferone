//
// Created by daniele on 15/11/23.
//

#pragma once

#include <memory>
#include <random>

namespace dferone::algorithms {

    template<class Solution>
    struct DestroyMethod {
        virtual ~DestroyMethod() = default;
        virtual void destroySolution(Solution &sol, std::mt19937 &mt) = 0;
        virtual std::unique_ptr<DestroyMethod> clone() const = 0;
    };
} // namespace dferone::algorithms