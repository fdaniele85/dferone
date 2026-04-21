//
// Shared infrastructure for metaheuristics.
//

#pragma once

#include <algorithm>
#include <format>
#include <functional>
#include <limits>
#include <mutex>
#include <random>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <dferone/tolerance.h>

namespace dferone::algorithms::detail {

    using LogFunction = std::function<void(std::string_view)>;

    inline std::vector<std::mt19937> make_thread_generators(std::mt19937 &generator, std::uint32_t num_threads) {
        std::vector<std::mt19937> generators;
        generators.reserve(num_threads);

        for (auto i = 0U; i < num_threads; ++i) {
            std::mt19937::result_type random_data[std::mt19937::state_size];
            auto next = [&generator]() { return generator(); };
            std::generate(std::begin(random_data), std::end(random_data), next);
            std::seed_seq seeds(std::begin(random_data), std::end(random_data));
            generators.emplace_back(seeds);
        }

        return generators;
    }

    inline std::vector<std::mt19937> make_thread_generators(unsigned int seed, std::uint32_t num_threads) {
        std::mt19937 generator(seed);
        return make_thread_generators(generator, num_threads);
    }

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
