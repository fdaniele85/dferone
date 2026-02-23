//
// Created by Daniele Ferone on 19/05/23.
//

#pragma once

#include "LocalSearch.h"
#include "SolutionConstructor.h"
#include <algorithm>
#include <concepts>
#include <dferone/cxxtimer.hpp>
#include <dferone/tolerance.h>
#include <functional>
#include <memory>
#include <random>
#include <thread>

namespace dferone::algorithms {
    /** @brief This class models the GRASP algorithm solver
     *
     *  @tparam ProblemInstance Class which represents an instance_ of the problem.
     *  @tparam Solution        Class which represents a solution.
     *                          It must implement the following methods:
     *                          * Solution(const ProblemInstance&) an empty-solution constructor.
     *                          * Solution(const Solution&) a copy constructor. Can be the implicit default.
     *                          * void operator=(const Solution& other) an assignment operator. Can be the implicit default.
     *                          * double get_cost() const, returning the cost of the solution (the smaller the better).
     */
    template<class ProblemInstance, std::copy_constructible Solution>
        requires requires(Solution s) {
            { s.get_cost() } -> std::convertible_to<double>;
            requires std::assignable_from<Solution &, Solution>;
        }
    class GRASP {
    public:
        using LogFunction = std::function<void(std::string_view)>;

        GRASP(const ProblemInstance &instance, unsigned int seed) : instance_(instance), generator_(seed), best_solution_(instance) {}

        /** @brief Add a Solution Costructor to construct a Solution at each GRASP iteration
         *
         * @param constructor SolutionConstructor<ProblemInstance, Solution> pointer
         */
        void add_solution_constructor(std::unique_ptr<SolutionConstructor<ProblemInstance, Solution>> &&constructor) { constructor_ = std::move(constructor); }

        /** @brief Add a Local search to improve a Solution at each GRASP iteration
         *
         * @param ls LocalSearch<Solution> pointer
         */
        void add_local_search(std::unique_ptr<LocalSearch<Solution>> &&ls) { ls_ = std::move(ls); }

        Solution solve(std::uint32_t num_threads) {
            if (!constructor_) {
                throw std::runtime_error("Cannot start GRASP without a constructor!");
            }

            if (max_iterations_ == 0 && max_seconds_ == 0 && target_ <= std::numeric_limits<double>::lowest()) {
                throw std::runtime_error("Stop condition not defined!");
            }

            timer_.start();

            start_threads(num_threads);

            timer_.stop();

            return best_solution_;
        }

        void set_max_iterations(std::size_t maxIterations) { max_iterations_ = maxIterations; }

        void set_max_seconds(std::size_t maxSeconds) { max_seconds_ = maxSeconds; }

        void set_target(double target) { target_ = target; }

        void set_tolerance(double eps) { tolerance_ = Tolerance(eps); }

        void set_logger(LogFunction logger, const int log_interval = 30) { logger_ = std::move(logger); log_interval_ = log_interval; }

        [[nodiscard]] double get_time() const { return timer_.elapsed(); }

         [[nodiscard]] double get_time_to_best() const { return time_to_best; }

    private:
        /*! @brief  Fire up a single thread.
         *
         *  @param   thread_id     Progressive id of the thread.
         */
        void start_thread(std::uint32_t thread_id, std::mt19937 &mt) {
            auto solution_constructor = constructor_->clone();
            std::unique_ptr<LocalSearch<Solution>> ls{nullptr};
            if (ls_) {
                ls = ls_->clone();
            }

            double best_thread_solution_cost = std::numeric_limits<double>::max();
            Solution best_thread_solution(instance_);

            unsigned int current_thread_iteration = 0;
            while (true) {
                ++current_thread_iteration;

                if (max_iterations_ > 0 && current_thread_iteration > max_iterations_) {
                    break;
                }

                if (max_seconds_ > 0 && timer_.elapsed() > max_seconds_) {
                    break;
                }

                if (best_thread_solution_cost <= target_) {
                    break;
                }

                auto s = solution_constructor->create_solution(instance_, mt);

                if (ls) {
                    ls->search(s, mt);
                }

                auto cost = s.get_cost();
                bool updated = false;
                if (tolerance_.less(cost, best_thread_solution_cost)) {
                    best_thread_solution = s;
                    best_thread_solution_cost = cost;
                    {
                        std::lock_guard _(best_solution_mutex_);
                        if (tolerance_.less(cost, best_solution_cost_)) {
                            best_solution_ = s;
                            best_solution_cost_ = cost;
                            time_to_best = timer_.elapsed();
                            updated = true;
                        }
                    }
                }


                if (logger_) {
                    std::lock_guard _(printing_mutex_);
                    auto elapsed = timer_.elapsed();
                    if (updated) {
                        logger_(std::format("Thread {}, time {}: updating best solution to {}", thread_id, elapsed, cost));
                        last_logged_time_ = elapsed;
                    } else if (last_logged_time_ + log_interval_ < elapsed) {
                        logger_(std::format("Thread {}, time {}: current best solution is {}", thread_id, elapsed, best_solution_cost_));
                        last_logged_time_ = elapsed;
                    }
                }
            }
        }

        /*! @brief  Fire up many threads.
         *
         *  @param   num_threads   Number of threads to start
         */
        void start_threads(std::uint32_t num_threads) {
            std::vector<std::mt19937> generators_;

            for (auto i = 0u; i < num_threads; ++i) {
                std::mt19937::result_type random_data[std::mt19937::state_size];
                auto g = [this]() { return generator_(); };
                std::generate(std::begin(random_data), std::end(random_data), g);
                std::seed_seq seeds(std::begin(random_data), std::end(random_data));
                generators_.emplace_back(seeds);
            }

            std::vector<std::jthread> threads(num_threads);
            for (auto i = 0u; i < num_threads; ++i) {
                threads[i] = std::jthread([i, &generators_, this]() { start_thread(i, generators_[i]); });
            }

            for (auto &thread : threads) {
                thread.join();
            }
        }

        /*! @brief Gets a solution's cost (but first locks the corresponding mutex).
         *
         *  @param sol       The solution of which we need the cost.
         *  @param sol_mutex The mutex protecting the solution.
         *  @return          The cost of the solution.
         */
        double threadsafe_get_solution_cost(const Solution &sol, std::mutex &sol_mutex) {
            std::lock_guard<std::mutex> _(sol_mutex);
            return sol.getCost();
        }

        /// Problem instance
        const ProblemInstance instance_;

        /// Generator
        mutable std::mt19937 generator_;

        /// Constructor to clone in each thread
        std::unique_ptr<SolutionConstructor<ProblemInstance, Solution>> constructor_{nullptr};

        /// Local search to clone in each thread
        std::unique_ptr<LocalSearch<Solution>> ls_{nullptr};

        /// Best solution found
        Solution best_solution_;

        double best_solution_cost_{std::numeric_limits<double>::max()};

        /// Maximum number of iterations (0 means infinity)
        std::size_t max_iterations_{0};

        /// Maximum seconds (0 means infinity)
        std::size_t max_seconds_{0};

        /// Target to reach
        double target_{std::numeric_limits<double>::lowest()};

        // Mutexes
        std::mutex best_solution_mutex_;

        std::mutex printing_mutex_;

        double last_logged_time_{0.0};

        cxxtimer::Timer timer_;

        /// Tolerance for comparing solution costs
        Tolerance tolerance_{1e-6};

        double time_to_best{0.0};

        LogFunction logger_;

        int log_interval_ { 30 };
    };
} // namespace dferone::algorithms