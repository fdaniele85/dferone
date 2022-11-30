//
// Created by daniele on 27/11/22.
//

#pragma once

#include <random>
#include <concepts>

/// Random namespace
namespace dferone::random {

    /** @brief  Gets a properly seeded Mersenne Twister prng.
    *
    *  @return A seeded instance of std::mt19937.
    */
    inline std::mt19937_64 get_seeded_mt() {
        std::array<std::mt19937_64 ::result_type,std::mt19937_64 ::state_size> random_data;
        std::random_device random_source;

        std::generate(random_data.begin(), random_data.end(), std::ref(random_source));

        std::seed_seq seeds(random_data.begin(), random_data.end());

        return std::mt19937_64(seeds);
    }

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
    template<std::floating_point FloatingPoint, class Prng = std::mt19937_64>
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


    template<std::ranges::range Container, class Random = std::mt19937_64>
    typename Container::const_iterator selectRandom(const Container &c, Random &rand) {
        assert(!c.empty());

        auto size = c.size();
        auto q = c.cbegin();

        std::uniform_int_distribution<uint> dis(0, size - 1);
        auto index = dis(rand);
        std::advance(q, index);
        return q;
    }

    template<std::forward_iterator Iterator, class Random = std::mt19937_64>
    Iterator selectRandom(Iterator q, std::size_t size, Random &rand) {
        assert(size > 0u);

        std::uniform_int_distribution<uint> dis(0, size - 1);
        auto index = dis(rand);
        std::advance(q, index);
        return q;
    }
}