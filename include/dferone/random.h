#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <iterator>
#include <random>
#include <ranges>
#include <vector>

namespace dferone::random {

    // ============================================================
    //  Concepts
    // ============================================================

    /// Uniform Random Bit Generator concept (e.g., std::mt19937_64)
    template<class Rng>
    concept URBG = std::uniform_random_bit_generator<Rng>;

    /// Range with size(), used for uniform random selection
    template<class R>
    concept SizedRange = std::ranges::range<R> && requires(const R &r) {
        { r.size() } -> std::convertible_to<std::size_t>;
    };

    // ============================================================
    //  Seeding utilities
    // ============================================================

    /// @brief Derive a deterministic set of MT generators from a single seed.
    ///
    /// Each generator receives a full seed sequence derived from the master
    /// generator initialized with @p seed.
    ///
    /// @param seed          master seed
    /// @param num_generators number of generators to create
    /// @return a vector of seeded std::mt19937 generators
    inline std::vector<std::mt19937> make_generators(unsigned int seed, std::size_t num_generators) {
        std::vector<std::mt19937> generators;
        generators.reserve(num_generators);

        std::mt19937 master(seed);
        for (auto i = 0U; i < num_generators; ++i) {
            std::mt19937::result_type random_data[std::mt19937::state_size];
            auto next = [&master]() { return master(); };
            std::generate(std::begin(random_data), std::end(random_data), next);
            std::seed_seq seq(std::begin(random_data), std::end(random_data));
            generators.emplace_back(seq);
        }

        return generators;
    }

    // ============================================================
    //  Uniform random selection (from container or iterator)
    // ============================================================

    /// @brief Select a random element uniformly from a sized range.
    /// @return Iterator to selected element.
    template<SizedRange Container, URBG Rng>
    auto random_select(const Container &c, Rng &rng) -> std::ranges::range_reference_t<const Container> {
        assert(!c.empty());
        std::uniform_int_distribution<std::size_t> dis(0, c.size() - 1);
        auto it = std::ranges::cbegin(c);
        std::advance(it, dis(rng));
        return *it;
    }

    /// @brief Select a random iterator uniformly from a sequence of given size.
    /// @param it starting iterator
    /// @param size number of valid elements starting from it
    /// @param rng random generator
    template<std::forward_iterator It, URBG Rng>
    It random_select(It it, const std::size_t size, Rng &rng) {
        assert(size > 0);
        std::uniform_int_distribution<std::size_t> dis(0, size - 1);
        std::advance(it, dis(rng));
        return it;
    }

    // ============================================================
    //  Geometric selection
    // ============================================================

    /// @brief Select an element randomly using a geometric distribution,
    ///        modulo the size of the container.
    ///
    /// Good when you want: "most picks near the start, but occasional far picks".
    template<std::ranges::random_access_range Container, URBG Rng>
    auto random_select_geometric(const Container &c, Rng &rng, const double p) {
        assert(!c.empty());
        std::geometric_distribution<int> gd(p);
        std::size_t idx = static_cast<std::size_t>(gd(rng)) % c.size();
        return std::ranges::cbegin(c) + idx;
    }

    // ============================================================
    //  Triangular distribution
    // ============================================================

    /// @brief Build a triangular distribution using piecewise_linear_distribution.
    /// @param a lower bound
    /// @param b mode (peak)
    /// @param c upper bound
    template<typename Real = double>
    auto triangular_distribution(double a, double b, double c) {
        std::array<double, 3> xs{a, b, c};
        std::array<double, 3> ws{0.0, 1.0, 0.0};
        return std::piecewise_linear_distribution<Real>(xs.begin(), xs.end(), ws.begin());
    }

} // namespace dferone::random
