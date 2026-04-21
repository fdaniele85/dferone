//
// Shared infrastructure for metaheuristics.
//

#pragma once

#include <format>
#include <functional>
#include <limits>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include <dferone/random.h>
#include <dferone/tolerance.h>

namespace dferone::algorithms::detail {

    using LogFunction = std::function<void(std::string_view)>;

    template<class Solution>
    class ConcurrentBestTracker {
    public:
        explicit ConcurrentBestTracker(Solution initial_solution, double initial_cost = std::numeric_limits<double>::max())
            : best_solution_(std::move(initial_solution)), best_solution_cost_(initial_cost) {}

        [[nodiscard]] Solution snapshot() const {
            std::lock_guard<std::mutex> _(mutex_);
            return best_solution_;
        }

        [[nodiscard]] double best_cost() const {
            std::lock_guard<std::mutex> _(mutex_);
            return best_solution_cost_;
        }

        [[nodiscard]] double time_to_best() const {
            std::lock_guard<std::mutex> _(mutex_);
            return time_to_best_;
        }

        void reset(const Solution &solution, double cost) {
            std::lock_guard<std::mutex> _(mutex_);
            best_solution_ = solution;
            best_solution_cost_ = cost;
            time_to_best_ = 0.0;
        }

        [[nodiscard]] bool update_if_better(const Solution &solution, double cost, const dferone::Tolerance &tolerance, double elapsed) {
            std::lock_guard<std::mutex> _(mutex_);
            if (!tolerance.less(cost, best_solution_cost_)) {
                return false;
            }

            best_solution_ = solution;
            best_solution_cost_ = cost;
            time_to_best_ = elapsed;
            return true;
        }

    private:
        mutable std::mutex mutex_;
        Solution best_solution_;
        double best_solution_cost_;
        double time_to_best_{0.0};
    };

    class PeriodicAlgorithmLogger {
    public:
        void set_logger(LogFunction logger, int log_interval = 30) {
            std::lock_guard<std::mutex> _(mutex_);
            logger_ = std::move(logger);
            log_interval_ = log_interval;
        }

        void reset() {
            std::lock_guard<std::mutex> _(mutex_);
            last_logged_time_ = 0.0;
        }

        void log_best_update(std::uint32_t thread_id, double elapsed, double cost) {
            std::lock_guard<std::mutex> _(mutex_);
            if (!logger_) {
                return;
            }

            const auto message = std::format("Thread {}, time {}: updating best solution to {}", thread_id, elapsed, cost);
            logger_(message);
            last_logged_time_ = elapsed;
        }

        void log_current_best(std::uint32_t thread_id, double elapsed, double cost) {
            std::lock_guard<std::mutex> _(mutex_);
            if (!logger_ || last_logged_time_ + log_interval_ >= elapsed) {
                return;
            }

            const auto message = std::format("Thread {}, time {}: current best solution is {}", thread_id, elapsed, cost);
            logger_(message);
            last_logged_time_ = elapsed;
        }

    private:
        std::mutex mutex_;
        LogFunction logger_;
        double last_logged_time_{0.0};
        int log_interval_{30};
    };

} // namespace dferone::algorithms::detail
