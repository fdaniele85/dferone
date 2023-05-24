//
// Created by daniele on 24/05/23.
//

#pragma once

#include <cmath>

/// \brief Implement the Welford's algorithm to calculate online the variance and the standard deviation
class WelfordAlgorithm {
public:
    /// \brief Add a value
    /// \param x Value to add
    void addElement(double x) {
        ++count_;
        double delta = x - mean_;
        mean_ += delta / static_cast<double>(count_);
        double delta2 = x - mean_;
        sum_of_squares_ += delta * delta2;
    }

    /// \return The mean
    [[nodiscard]] double getMean() const {
        return mean_;
    }

    /// \return The variance
    [[nodiscard]] double getVariance() const {
        return sum_of_squares_ / static_cast<double>(count_);
    }

    /// \return The standard deviation
    [[nodiscard]] double getStdDev() const {
        return std::sqrt(getVariance());
    }

private:
    std::size_t count_{0};
    double mean_{0.0};
    double sum_of_squares_{0.0};
};