//
// Created by daniele on 27/11/22.
//

#pragma once

#include <random>
#include <concepts>
#include <array>
#include <cassert>
#include <functional>

/// Random namespace
namespace dferone::random {

    /** @brief  Gets a properly seeded Mersenne Twister prng.
    *
    *  @return A seeded instance of std::mt19937.
    */
    inline std::mt19937_64 get_seeded_mt() {
        std::array<std::mt19937_64::result_type, std::mt19937_64::state_size> random_data;
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
            const std::vector<FloatingPoint> &weights,
            Prng &&prng
    ) {
        // Roulette wheel selection only works if all weights are non-negative.
        assert(std::all_of(weights.begin(), weights.end(), [](auto w) { return w >= 0; }));

        // There needs to be at least one element in the vector.
        assert(!weights.empty());

        const FloatingPoint sum = std::accumulate(weights.begin(), weights.end(), 0.0f);
        std::uniform_real_distribution<FloatingPoint> dist(0, sum);
        const FloatingPoint pivot = dist(prng);

        FloatingPoint partial = 0;

        for (typename std::vector<FloatingPoint>::size_type i = 0u; i < weights.size() - 1u; ++i) {
            partial += weights[i];

            if (partial >= pivot) {
                return i;
            }
        }

        return weights.size() - 1u;
    }


    /// Randomly select an element from a container with uniform distribution
    /// \tparam Container Container type
    /// \tparam Random Random number generator
    /// \param c The container
    /// \param rand The number generator
    /// \return an element of the container
    template<std::ranges::range Container, class Random = std::mt19937_64>
    typename Container::const_iterator selectRandom(const Container &c, Random &rand)requires requires {
        { c.size() } -> std::same_as<typename Container::size_type>;
    } {
        assert(!c.empty());

        auto size = c.size();
        auto q = c.cbegin();

        std::uniform_int_distribution<uint> dis(0, size - 1);
        auto index = dis(rand);
        std::advance(q, index);
        return q;
    }

    /// Select a random element given an iterator and the number of elements with uniform distribution
    /// \tparam Iterator Iterator type (at least std::forward_iterator)
    /// \tparam Random Random number generator type
    /// \param q The initial iterator
    /// \param size The number of valid iterators
    /// \param rand The random number generator
    /// \return An iterator to the selected element
    template<std::forward_iterator Iterator, class Random = std::mt19937_64>
    Iterator selectRandom(Iterator q, std::size_t size, Random &rand) {
        assert(size > 0u);

        std::uniform_int_distribution<uint> dis(0, size - 1);
        auto index = dis(rand);
        std::advance(q, index);
        return q;
    }

    /// Randomly select an element from a container with geometric distribution
    /// \tparam Container Container type
    /// \tparam Random Random number generator
    /// \param c The container
    /// \param rand The number generator
    /// \param distribution_parameter The parameter of the geometric distribution
    /// \return an element of the container
    template<std::ranges::random_access_range Container, class Random = std::mt19937_64>
    typename Container::const_iterator
    selectRandom(const Container &c, Random &rand, double distribution_parameter)requires requires {
        { c.size() } -> std::same_as<typename Container::size_type>;
    } {
        assert(!c.empty());

        auto size = c.size();
        auto q = c.cbegin();

        std::geometric_distribution<int> gd(distribution_parameter);
        auto l = gd(rand);
        auto index = static_cast<uint>(l % size);

        std::advance(q, index);
        return q;
    }

    /// \brief Creates a triangular distribution with the use of std::piecewise_linear_distribution
    /// \tparam RealType The result type generated by the generator. The effect is undefined if this is not one of float, double, or long double.
    /// \param min Lower limit
    /// \param peak Mode
    /// \param max Upper limit
    /// \return A triangular distribution
    template <typename RealType = double>
    std::piecewise_linear_distribution<RealType> getTriangularDistribution(double min, double peak, double max) {
        std::array<double, 3> i{min, peak, max};
        std::array<double, 3> w{0, 1, 0};
        return std::piecewise_linear_distribution<RealType>{i.begin(), i.end(), w.begin()};
    }
}