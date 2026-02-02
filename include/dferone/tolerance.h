//
// Created by daniele on 02/02/26.
//

#pragma once

#include <cmath>

namespace dferone {

    class Tolerance {
    public:
        explicit Tolerance(const double epsilon = 1e-9) : epsilon_(epsilon) {}

        [[nodiscard]] bool equal(const double a, const double b) const { return std::abs(a - b) <= epsilon_ * std::max({1.0, std::abs(a), std::abs(b)}); }

        [[nodiscard]] bool less(const double a, const double b) const { return b - a > epsilon_ * std::max({1.0, std::abs(a), std::abs(b)}); }

        [[nodiscard]] bool less_or_equal(const double a, const double b) const { return less(a, b) || equal(a, b); }

        [[nodiscard]] bool greater(const double a, const double b) const { return less(b, a); }

        [[nodiscard]] bool greater_or_equal(const double a, const double b) const { return greater(a, b) || equal(a, b); }

    private:
        double epsilon_;
    };

} // namespace dferone
