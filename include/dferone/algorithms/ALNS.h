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
#include <dferone/cxxtimer.hpp>
#include <dferone/random.h>
#include <dferone/tolerance.h>
#include <format>

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
        using LogFunction = std::function<void(std::string_view)>;

        explicit ALNS(const ProblemInstance &instance, const Parameters &parameters)
            : instance_(instance), parameters_(parameters), best_solution_(instance) {}

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

            return threadsafe_clone_solution(best_solution_, best_solution_mutex_);
        }

        Solution search(const std::uint32_t num_threads) {
            Solution start_solution(instance_);
            return search(start_solution, num_threads);
        }

        void set_visitor(std::unique_ptr<AlgorithmVisitor<Solution>> &&visitor) { visitor_ = std::move(visitor); }

        void set_tolerance(double eps) { tolerance_ = dferone::Tolerance(eps); }

        void set_logger(LogFunction logger, const int log_interval = 30) { logger_ = std::move(logger); log_interval_ = log_interval; }

        [[nodiscard]] double get_time() const { return timer_.elapsed(); }

        [[nodiscard]] double get_time_to_best() const { return time_to_best; }

    private:
        static Solution threadsafe_clone_solution(const Solution &sol, std::mutex &sol_mutex) {
            std::lock_guard<std::mutex> _(sol_mutex);
            return sol;
        }

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

                    {
                        std::lock_guard _(best_solution_mutex_);
                        if (tolerance_.less(new_sol_cost, best_solution_cost_)) {
                            best_solution_ = tmp_sol;
                            best_solution_cost_ = new_sol_cost;
                            time_to_best = elapsed;
                            new_global_best = true;
                        }
                    }
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
                    notify_new_best(tmp_sol, new_sol_cost, elapsed);
                    log_best_update(thread_id, elapsed, new_sol_cost);
                } else {
                    log_current_best(thread_id, elapsed);
                }
            }
        }

        void log_best_update(std::uint32_t thread_id, double elapsed, double cost) {
            if (!logger_) {
                return;
            }

            std::lock_guard _(printing_mutex_);
            logger_(std::format("Thread {}, time {}: updating best solution to {}", thread_id, elapsed, cost));
            last_logged_time_ = elapsed;
        }

        void log_current_best(std::uint32_t thread_id, double elapsed) {
            if (!logger_) {
                return;
            }

            std::lock_guard _(printing_mutex_);
            if (last_logged_time_ + log_interval_ >= elapsed) {
                return;
            }

            double best_cost;
            {
                std::lock_guard _(best_solution_mutex_);
                best_cost = best_solution_cost_;
            }
            logger_(std::format("Thread {}, time {}: current best solution is {}", thread_id, elapsed, best_cost));
            last_logged_time_ = elapsed;
        }

        void notify_new_best(const Solution &solution, double cost, double current_time) {
            if (!visitor_) {
                return;
            }

            std::lock_guard _(visitor_mutex_);
            visitor_->on_new_best(solution, cost, current_time);
        }

        void start_threads(std::uint32_t num_threads) {
            if (num_threads == 0) {
                return;
            }

            const auto start_solution = threadsafe_clone_solution(best_solution_, best_solution_mutex_);
            auto generators = dferone::random::make_generators(parameters_.seed, num_threads);
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

        Solution best_solution_;
        double best_solution_cost_{std::numeric_limits<double>::max()};

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
            {
                std::lock_guard _(best_solution_mutex_);
                best_solution_ = solution;
                best_solution_cost_ = solution.get_cost();
            }
            time_to_best = 0.0;
            last_logged_time_ = 0.0;
        }

        std::unique_ptr<AlgorithmVisitor<Solution>> visitor_{nullptr};

        dferone::Tolerance tolerance_{1e-6};
        LogFunction logger_;
        int log_interval_{30};
        std::mutex best_solution_mutex_;
        std::mutex printing_mutex_;
        double last_logged_time_{0.0};
        double time_to_best{0.0};
        std::mutex visitor_mutex_;
    };

} // namespace dferone::algorithms
