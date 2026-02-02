#pragma once

#include <random>
#include <array>
#include <vector>
#include <ranges>
#include <cassert>
#include <concepts>
#include <iterator>
#include <algorithm>

namespace dferone::random {

// ============================================================
//  Concepts
// ============================================================

/// Uniform Random Bit Generator concept (e.g., std::mt19937_64)
template<class Rng>
concept URBG = std::uniform_random_bit_generator<Rng>;

/// Range with size(), used for uniform random selection
template<class R>
concept SizedRange =
    std::ranges::range<R> &&
    requires(const R& r) {
        { r.size() } -> std::convertible_to<std::size_t>;
    };

// ============================================================
//  Seeding utilities
// ============================================================

/// @brief Create a strongly-seeded Mersenne Twister 64-bit PRNG.
/// 
/// Uses std::random_device to fill the entire MT state via seed_seq.
/// This is stronger than seeding with a single integer.
/// 
/// @return a seeded std::mt19937_64 instance.
inline std::mt19937_64 make_seeded_mt() {
    std::array<std::mt19937_64::result_type,
               std::mt19937_64::state_size> entropy{};

    std::random_device rd;
    std::ranges::generate(entropy, std::ref(rd));

    std::seed_seq seq(entropy.begin(), entropy.end());
    return std::mt19937_64(seq);
}

// ============================================================
//  Uniform random selection (from container or iterator)
// ============================================================

/// @brief Select a random element uniformly from a sized range.
/// @return Iterator to selected element.
template<SizedRange Container, URBG Rng>
auto random_select(const Container& c, Rng& rng) {
    assert(!c.empty());
    std::uniform_int_distribution<std::size_t> dis(0, c.size() - 1);
    auto it = std::ranges::cbegin(c);
    std::advance(it, dis(rng));
    return it;
}

/// @brief Select a random iterator uniformly from a sequence of given size.
/// @param it starting iterator
/// @param size number of valid elements starting from it
/// @param rng random generator
template<std::forward_iterator It, URBG Rng>
It random_select(It it, const std::size_t size, Rng& rng) {
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
auto random_select_geometric(const Container& c, Rng& rng, const double p) {
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
    std::array<double, 3> xs { a, b, c };
    std::array<double, 3> ws { 0.0, 1.0, 0.0 };
    return std::piecewise_linear_distribution<Real>(xs.begin(), xs.end(), ws.begin());
}

} // namespace dferone::random
