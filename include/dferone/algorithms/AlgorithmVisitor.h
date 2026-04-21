//
// Created by alberto on 28/06/17.
//

#pragma once

namespace dferone::algorithms {
    /**
     * This class implements a Visitor for the algorithm, to be called when a
     * new global best solution is found.
     *
     * @tparam Solution The solution type.
     */
    template<class Solution>
    struct AlgorithmVisitor {
        /**
         * This method is called when a new global best solution is found.
         *
         * @param solution      The new best solution.
         * @param cost          The cost of the new best solution.
         * @param current_time  The elapsed execution time when the solution was found.
         */
        virtual void on_new_best(const Solution &solution, double cost, double current_time) = 0;

        /**
         * Virtual destructor.
         */
        virtual ~AlgorithmVisitor() = default;
    };
} // namespace dferone::algorithms
