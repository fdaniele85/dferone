//
// Created by daniele on 24/05/23.
//

#pragma once

#include <cstdint>

namespace dferone::grasp {
    template<class Solution>
    struct AlgorithmStatus {
        // New solution produced at this iter
        Solution& new_solution_;

        // Global best solution
        Solution& best_solution_;

        // Global iteration count
        std::size_t iteration_;

        // Whether the new sol was the new global best sol
        bool new_best_;

        AlgorithmStatus(Solution& new_solution, Solution& best_solution)
                : new_solution_{new_solution}, best_solution_{best_solution} {}
    };
} // namespace dferone::grasp