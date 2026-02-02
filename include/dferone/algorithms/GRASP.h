//
// Created by Daniele Ferone on 19/05/23.
//

#pragma once

#include "AlgorithmStatus.h"
#include "AlgorithmVisitor.h"
#include "LocalSearch.h"
#include "SolutionConstructor.h"
#include <algorithm>
#include <concepts>
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
     *                          * double getCost() const, returning the cost of the solution (the smaller the better).
     */
    template<class ProblemInstance, std::copy_constructible Solution>
        requires requires(Solution s) {
            { s.getCost() } -> std::convertible_to<double>;
            std::assignable_from<Solution, Solution>;
        }
    class GRASP {
    public:
        GRASP(const ProblemInstance &instance, unsigned int seed) : instance_(instance), generator_(seed), best_solution_(instance) {}

        /** @brief Add a Solution Costructor to construct a Solution at each GRASP iteration
         *
         * @param constructor SolutionConstructor<ProblemInstance, Solution> pointer
         */
        void addSolutionConstructor(std::unique_ptr<SolutionConstructor<ProblemInstance, Solution>> &&constructor) { constructor_ = std::move(constructor); }

        /** @brief Add a Local search to improve a Solution at each GRASP iteration
         *
         * @param ls LocalSearch<Solution> pointer
         */
        void addLocalSearch(std::unique_ptr<LocalSearch<Solution>> &&ls) { ls_ = std::move(ls); }

        Solution solve(std::uint32_t num_threads) {
            if (!constructor_) {
                throw std::runtime_error("Cannot start GRASP without a constructor!");
            }

            if (max_iterations_ == 0 && max_seconds_ == 0 && target_ <= std::numeric_limits<double>::min()) {
                throw std::runtime_error("Stop condition not defined!");
            }

            auto old_max_iterations = max_iterations_;
            max_iterations_ = static_cast<unsigned int>(std::ceil(static_cast<double>(max_iterations_) / num_threads));

            start_time_ = std::chrono::high_resolution_clock::now();

            if (visitor_) {
                visitor_->on_algorithm_start();
            }

            start_threads(num_threads);

            max_iterations_ = old_max_iterations;

            return best_solution_;
        }

        void setMaxIterations(std::size_t maxIterations) { max_iterations_ = maxIterations; }

        void setMaxSeconds(std::size_t maxSeconds) { max_seconds_ = maxSeconds; }

        void setTarget(double target) { target_ = target; }

        void addVisitor(std::unique_ptr<AlgorithmVisitor<Solution>> &&visitor) { visitor_ = std::move(visitor); }

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

            unsigned int current_thread_iteration = 0;
            while (true) {
                ++current_thread_iteration;
                auto global_iteration = 0;
                {
                    std::lock_guard _(current_iteration_mutex_);
                    global_iteration = ++current_iteration_;
                }

                if (max_iterations_ > 0 && current_thread_iteration > max_iterations_) {
                    break;
                }

                if (max_seconds_ > 0) {
                    // Calculate elapsed time
                    auto current_time = std::chrono::high_resolution_clock::now();
                    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - start_time_).count();
                    if (elapsed_time > max_seconds_) {
                        break;
                    }
                }

                auto c = threadsafe_get_solution_cost(best_solution_, best_solution_mutex_);
                if (c <= target_) {
                    break;
                }

                auto s = solution_constructor->createSolution(instance_, mt);
                auto new_best = updateBestSolution(s);
                AlgorithmStatus<Solution> status(s, best_solution_);
                status.new_best_ = new_best;
                status.iteration_ = global_iteration;

                auto perform_ls = true;
                if (visitor_) {
                    // Visitor can modify best_solution
                    std::lock_guard _(best_solution_mutex_);
                    perform_ls = visitor_->on_construction_end(status);
                }

                if (ls) {
                    ls->search(s, mt);
                }

                status.new_best_ = updateBestSolution(s) || new_best;
                if (visitor_) {
                    // Visitor can modify best_solution
                    std::lock_guard _(best_solution_mutex_);
                    visitor_->on_iteration_end(status);
                }

                if (status.new_best_) {
                    std::lock_guard _(printing_mutex_);
                    LOG(INFO) << "Iteration " << current_iteration_ << ": updating best solution to " << status.best_solution_.getCost();
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

        /** @brief Checks if the best solution must be updated
         *
         * @param new_sol New solution to check
         * @return True if the best solution has been updated, false otherwise
         */
        bool updateBestSolution(const Solution &new_sol) {
            auto cost = new_sol.getCost();

            std::lock_guard _(best_solution_mutex_);
            auto best_cost = best_solution_.getCost();
            if (cost < best_cost - eps_) {
                best_solution_ = new_sol;
                return true;
            }
            return false;
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

        /// Current iteration
        std::size_t current_iteration_{0};

        /// Maximum number of iterations (0 means infinity)
        std::size_t max_iterations_{0};

        /// Maximum seconds (0 means infinity)
        std::size_t max_seconds_{0};

        /// Target to reach
        double target_{std::numeric_limits<double>::lowest()};

        // Mutexes
        std::mutex best_solution_mutex_;

        std::mutex printing_mutex_;

        std::mutex current_iteration_mutex_;

        std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;

        /*! @brief Precision to use when comparing solution scores. */
        static constexpr double eps_ = 1e-6;

        /// Visitor
        std::unique_ptr<AlgorithmVisitor<Solution>> visitor_{nullptr};
    };
} // namespace dferone::algorithms