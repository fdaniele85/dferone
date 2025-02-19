//
// Created by alberto on 28/06/17.
//

#pragma once

#include "AlgorithmStatus.h"
#include <string>
#include <vector>

namespace dferone::algorithms {
    /**
     * This class implements a Visitor for the algorithm, to be called at
     * specific points during the solution process.
     *
     * @tparam Solution The solution type.
     */
    template<class Solution>
    struct AlgorithmVisitor {
        /**
         * This method is called at the start of the algorithm, before the first iteration.
         */
        virtual void on_algorithm_start() = 0;

        /**
         * This method is called at the end of every construction.
         *
         * @param alg_status An AlgorithmStatus object containing info on the current state of the algorithm.
         * @return true      If the local search must be performed
         */
        virtual bool on_construction_end(AlgorithmStatus<Solution>& alg_status) = 0;

        /**
         * This method is called at the end of every iteration.
         *
         * @param alg_status    An AlgorithmStatus object containing info on the current state of the algorithm.
         */
        virtual void on_iteration_end(AlgorithmStatus<Solution>& alg_status) = 0;

        /**
         * Virtual destructor.
         */
        virtual ~AlgorithmVisitor() = default;
    };
} // namespace dferone::algorithms
