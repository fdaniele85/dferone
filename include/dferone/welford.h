//
// Created by daniele on 24/05/23.
//

#pragma once

#include <cmath>

namespace dferone {

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
        [[nodiscard]] double get_mean() const { return mean_; }

        /// \return The variance
        [[nodiscard]] double get_variance() const { return count_ > 0 ? sum_of_squares_ / static_cast<double>(count_) : 0.0; }

        /// \return The standard deviation
        [[nodiscard]] double get_std_dev() const { return count_ > 0 ? std::sqrt(get_variance()) : 0.0; }

    private:
        std::size_t count_{0};
        double mean_{0.0};
        double sum_of_squares_{0.0};
    };

} // namespace dferone