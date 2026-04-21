//
// Created by daniele on 30/10/23.
//

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

#include "AlgorithmVisitor.h"
#include "DestroyMethod.h"
#include "RepairMethod.h"
#include "detail/MetaheuristicCommon.h"
#include <dferone/cxxtimer.hpp>
#include <dferone/tolerance.h>

namespace dferone::algorithms {

    /** @brief This class models the ALNS algorithm solver
     *
     *  @tparam ProblemInstance Class which represents an instance_ of the problem.
     *  @tparam Solution        Class which represents a solution.
     *                          It must implement the following methods:
     *                          * Solution(const ProblemInstance&) an empty-solution constructor.
     *                          * Solution(const Solution&) a copy constructor. Can be the implicit default.
     *                          * void operator=(const Solution& other) an assignment operator. Can be the implicit default.
     *                          * double get_cost() const, returning the cost of the solution (the smaller the better).
     */
    template<class ProblemInstance, std::copy_constructible Solution, class Parameters>
        requires requires(Solution s) {
            { s.get_cost() } -> std::convertible_to<double>;
            requires std::assignable_from<Solution &, Solution>;
        }
    class ALNS {
    public:
        using DestroyMethodPtr = std::unique_ptr<DestroyMethod<Solution>>;
        using RepairMethodPtr = std::unique_ptr<RepairMethod<Solution>>;
        using LogFunction = detail::LogFunction;

        explicit ALNS(const ProblemInstance &instance, const Parameters &parameters)
            : instance_(instance), parameters_(parameters), best_solution_(Solution(instance)) {}

        /// \brief Add the destroy method to the list of destroy methods
        /// \param destroy_method The destroy method to add
        /// \return The index of the newly added method in the destroy methods vector
        std::size_t add_destroy_method(DestroyMethodPtr &&destroy_method) {
            destroy_methods_.push_back(std::move(destroy_method));
            return destroy_methods_.size() - 1;
        }

        /// \brief Add the repair method to the list of destroy methods
        /// \param repair_method The repair method to add
        /// \return The index of the newly added method in the repair methods vector
        std::size_t add_repair_method(RepairMethodPtr &&repair_method) {
            repairs_methods_.push_back(std::move(repair_method));
            return repairs_methods_.size() - 1;
        }

        Solution search(const Solution &start_solution, std::uint32_t num_threads) {
            if (destroy_methods_.empty() || repairs_methods_.empty()) {
                reset_parameters(start_solution, 0);
                return start_solution;
            }

            if (parameters_.max_iterations == 0 && parameters_.max_time <= 0.0) {
                throw std::runtime_error("Cannot start ALNS without a stop condition!");
            }

            reset_parameters(start_solution, num_threads);
            timer_.reset();
            timer_.start();
            start_threads(num_threads);
            timer_.stop();

            return best_solution_.snapshot();
        }

        Solution search(const std::uint32_t num_threads) {
            Solution start_solution(instance_);
            return search(start_solution, num_threads);
        }

        void set_visitor(std::unique_ptr<AlgorithmVisitor<Solution>> &&visitor) { visitor_ = std::move(visitor); }

        void set_tolerance(double eps) { tolerance_ = dferone::Tolerance(eps); }

        void set_logger(LogFunction logger, const int log_interval = 30) { logger_.set_logger(std::move(logger), log_interval); }

        [[nodiscard]] double get_time() const { return timer_.elapsed(); }

        [[nodiscard]] double get_time_to_best() const { return best_solution_.time_to_best(); }

    private:
        static int roulette_wheel_selection(const std::vector<double> &weights, std::mt19937 &mt) {
            std::discrete_distribution<int> dist(weights.begin(), weights.end());
            return dist(mt);
        }

        void start_thread(Solution current_solution, const std::uint32_t thread_id, std::mt19937 &mt) {
            std::vector<DestroyMethodPtr> local_destroy_methods;
            std::vector<RepairMethodPtr> local_repair_methods;
            local_destroy_methods.reserve(destroy_methods_.size());
            local_repair_methods.reserve(repairs_methods_.size());

            for (const auto &destroy_method : destroy_methods_) {
                local_destroy_methods.push_back(destroy_method->clone());
            }

            for (const auto &repair_method : repairs_methods_) {
                local_repair_methods.push_back(repair_method->clone());
            }

            std::vector<double> destroy_weights(local_destroy_methods.size(), 1.0);
            std::vector<double> repair_weights(local_repair_methods.size(), 1.0);

            auto local_temperature = parameters_.initial_temperature;
            const auto local_score_decay = parameters_.score_decay;
            const auto local_cooling_rate = parameters_.cooling_rate;
            const auto local_max_iterations = parameters_.max_iterations;
            const auto local_max_time = parameters_.max_time;

            auto current_solution_cost = current_solution.get_cost();
            Solution tmp_sol = current_solution;
            auto best_thread_solution_cost = current_solution_cost;

            auto iterations = 0U;
            while (true) {
                ++iterations;
                if (local_max_iterations > 0 && iterations > local_max_iterations) {
                    break;
                }

                if (local_max_time > 0.0 && timer_.elapsed() > local_max_time) {
                    break;
                }

                const auto chosen_destroy_id = static_cast<std::size_t>(roulette_wheel_selection(destroy_weights, mt));
                const auto chosen_repair_id = static_cast<std::size_t>(roulette_wheel_selection(repair_weights, mt));

                local_destroy_methods[chosen_destroy_id]->destroySolution(tmp_sol, mt);
                local_repair_methods[chosen_repair_id]->repairSolution(tmp_sol, mt);

                const auto new_sol_cost = tmp_sol.get_cost();
                const auto improved = tolerance_.less(new_sol_cost, current_solution_cost);
                const auto accepted =
                    improved || should_accept(current_solution_cost, new_sol_cost, tmp_sol, mt, local_temperature);

                bool new_global_best = false;
                const auto elapsed = timer_.elapsed();
                if (accepted) {
                    current_solution = tmp_sol;
                    current_solution_cost = new_sol_cost;
                    if (tolerance_.less(new_sol_cost, best_thread_solution_cost)) {
                        best_thread_solution_cost = new_sol_cost;
                    }

                    new_global_best = best_solution_.update_if_better(tmp_sol, new_sol_cost, tolerance_, elapsed);
                } else {
                    tmp_sol = current_solution;
                }

                const auto new_score = calculate_score(accepted, improved, new_global_best);
                destroy_weights[chosen_destroy_id] =
                    destroy_weights[chosen_destroy_id] * local_score_decay + new_score * (1.0 - local_score_decay);
                repair_weights[chosen_repair_id] =
                    repair_weights[chosen_repair_id] * local_score_decay + new_score * (1.0 - local_score_decay);

                local_temperature *= local_cooling_rate;

                if (new_global_best) {
                    logger_.log_best_update(thread_id, elapsed, new_sol_cost);
                } else {
                    logger_.log_current_best(thread_id, elapsed, best_solution_.best_cost());
                }
            }
        }

        void start_threads(std::uint32_t num_threads) {
            if (num_threads == 0) {
                return;
            }

            const auto start_solution = best_solution_.snapshot();
            auto generators = detail::make_thread_generators(parameters_.seed, num_threads);
            std::vector<std::jthread> threads(num_threads);
            for (auto i = 0U; i < num_threads; ++i) {
                threads[i] = std::jthread([this, i, start_solution, &generators]() {
                    start_thread(start_solution, i, generators[i]);
                });
            }

            for (auto &thread : threads) {
                thread.join();
            }
        }

        const ProblemInstance &instance_;
        const Parameters &parameters_;

        detail::ConcurrentBestTracker<Solution> best_solution_;

        std::vector<DestroyMethodPtr> destroy_methods_;
        std::vector<RepairMethodPtr> repairs_methods_;

        /// \brief Timer of the algorithm
        cxxtimer::Timer timer_;

        bool should_accept(double current_cost, double new_cost, const Solution &, std::mt19937 &mt, double temperature) {
            if (tolerance_.less(new_cost, current_cost)) {
                return true;
            }

            if (!tolerance_.positive(temperature)) {
                return false;
            }

            std::uniform_real_distribution<double> dist(0.0, 1.0);
            const auto acceptance_probability = std::exp((current_cost - new_cost) / temperature);
            return dist(mt) < acceptance_probability;
        }

        /// \brief  Calculates the score of a destroy/repair method which produced a new solution that was accepted / improved on current / improved on best.
        /// \param   is_accepted           The produced solution was accepted?
        /// \param   is_improved           The produced solution improved on the current?
        /// \param   is_new_global_best    The produced solution improved on the best?
        /// \return                        The new score.
        [[nodiscard]] float calculate_score(bool is_accepted, bool is_improved, bool is_new_global_best) const {
            if (is_new_global_best) {
                return parameters_.new_best_score;
            }

            if (is_improved) {
                return parameters_.improved_score;
            }

            if (is_accepted) {
                return parameters_.accepted_score;
            }

            return 0.0F;
        }

        /// \brief Reset the internal parameters
        void reset_parameters(const Solution &solution, std::uint32_t num_threads) {
            static_cast<void>(num_threads);
            best_solution_.reset(solution, solution.get_cost());
            logger_.reset();
        }

        std::unique_ptr<AlgorithmVisitor<Solution>> visitor_{nullptr};

        dferone::Tolerance tolerance_{1e-6};
        detail::PeriodicAlgorithmLogger logger_;
    };

} // namespace dferone::algorithms
