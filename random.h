//
// Created by daniele on 27/11/22.
//

#pragma once

#include <random>
#include <concepts>

namespace dferone::random {
    /** @brief  Selects a position in a vector of floating-point numbers according
 *          to a roulette-wheel criterion.
 *
 *          In a roulette-wheel random choice, each position has a probability
 *          to be chosen proportional to the weight at that position.
 *
 *  @tparam FloatingPoint   A floating-point type (e.g. float, double).
 *  @tparam Prng            The type of the pseudo-random number generator.
 *  @param  weights         The non-empty vector with weights.
 *  @param  prng            The pseudo-random number generator.
 *  @return                 An index of the vector of \ref weights.
 */
    template<class FloatingPoint, class Prng = std::mt19937> requires std::floating_point<FloatingPoint>
    inline typename std::vector<FloatingPoint>::size_type roulette_wheel(
            const std::vector<FloatingPoint>& weights,
            Prng&& prng
    ) {
        // Roulette wheel selection only works if all weights are non-negative.
        assert(std::all_of(weights.begin(), weights.end(), [](auto w){return w >= 0;}));

        // There needs to be at least one element in the vector.
        assert(!weights.empty());

        const FloatingPoint sum = std::accumulate(weights.begin(), weights.end(), 0.0f);
        std::uniform_real_distribution<FloatingPoint> dist(0, sum);
        const FloatingPoint pivot = dist(prng);

        FloatingPoint partial = 0;

        for(typename std::vector<FloatingPoint>::size_type i = 0u; i < weights.size() - 1u; ++i) {
            partial += weights[i];

            if(partial >= pivot) {
                return i;
            }
        }

        return weights.size() - 1u;
    }
}