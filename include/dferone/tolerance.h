//
// Created by daniele on 02/02/26.
//

#pragma once

#include <algorithm>
#include <cmath>

namespace dferone {

    class Tolerance {
    public:
        explicit Tolerance(const double epsilon = 1e-9) : epsilon_(epsilon) {}

        [[nodiscard]] bool equal(const double a, const double b) const { return !different(a, b); }

        [[nodiscard]] bool less(const double a, const double b) const { return a + epsilon_ < b; }

        [[nodiscard]] bool less_or_equal(const double a, const double b) const { return less(a, b) || equal(a, b); }

        [[nodiscard]] bool greater(const double a, const double b) const { return less(b, a); }

        [[nodiscard]] bool greater_or_equal(const double a, const double b) const { return greater(a, b) || equal(a, b); }

        [[nodiscard]] bool different(const double a, const double b) const { return less(a, b) || less(b, a); }

        [[nodiscard]] bool positive(const double a) const { return greater(a, 0); }

        [[nodiscard]] bool negative(const double a) const { return less(a, 0); }

    private:
        double epsilon_;
    };

} // namespace dferone
