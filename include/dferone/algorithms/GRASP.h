//
// Created by Daniele Ferone on 19/05/23.
//

#pragma once

#include "LocalSearch.h"
#include "SolutionConstructor.h"
#include "detail/MetaheuristicCommon.h"
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
        using LogFunction = detail::LogFunction;

        GRASP(const ProblemInstance &instance, unsigned int seed) : instance_(instance), generator_(seed), best_solution_(Solution(instance)) {}

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

            return best_solution_.snapshot();
        }

        void set_max_iterations(std::size_t maxIterations) { max_iterations_ = maxIterations; }

        void set_max_seconds(std::size_t maxSeconds) { max_seconds_ = maxSeconds; }

        void set_target(double target) { target_ = target; }

        void set_tolerance(double eps) { tolerance_ = Tolerance(eps); }

        void set_logger(LogFunction logger, const int log_interval = 30) { logger_.set_logger(std::move(logger), log_interval); }

        [[nodiscard]] double get_time() const { return timer_.elapsed(); }

         [[nodiscard]] double get_time_to_best() const { return best_solution_.time_to_best(); }

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
                    best_thread_solution_cost = cost;
                    updated = best_solution_.update_if_better(s, cost, tolerance_, timer_.elapsed());
                }


                const auto elapsed = timer_.elapsed();
                if (updated) {
                    logger_.log_best_update(thread_id, elapsed, cost);
                } else {
                    logger_.log_current_best(thread_id, elapsed, best_solution_.best_cost());
                }

            }
        }

        /*! @brief  Fire up many threads.
         *
         *  @param   num_threads   Number of threads to start
         */
        void start_threads(std::uint32_t num_threads) {
            auto generators_ = detail::make_thread_generators(generator_, num_threads);

            std::vector<std::jthread> threads(num_threads);
            for (auto i = 0u; i < num_threads; ++i) {
                threads[i] = std::jthread([i, &generators_, this]() { start_thread(i, generators_[i]); });
            }

            for (auto &thread : threads) {
                thread.join();
            }
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
        detail::ConcurrentBestTracker<Solution> best_solution_;

        /// Maximum number of iterations (0 means infinity)
        std::size_t max_iterations_{0};

        /// Maximum seconds (0 means infinity)
        std::size_t max_seconds_{0};

        /// Target to reach
        double target_{std::numeric_limits<double>::lowest()};

        cxxtimer::Timer timer_;

        /// Tolerance for comparing solution costs
        Tolerance tolerance_{1e-6};

        detail::PeriodicAlgorithmLogger logger_;
    };
} // namespace dferone::algorithms
