/*
 * GRASP.h
 *
 *  Created on: 19 giu 2016
 *      Author: daniele
 */

#ifndef GRASP_H_
#define GRASP_H_

#ifdef _OPENMP
#include <omp.h>
#endif

#include <iostream>
#include <boost/timer/timer.hpp>
#include <random>
#include <easylogging++.h>


#include "Solver.h"
#include "implementation/PathRelink.h"

#include "../containers/BestSet.h"

namespace dferone {
namespace metaheuristics {

template<typename Instance, typename Solution, typename Constructor,
		typename LocalSearch, typename PathRelinking = implementation::PathRelink<Solution>,
		typename Comparator = std::less<Solution>>
class GRASP: public Solver<Solution, Instance> {
public:

	GRASP(const Instance &instance, unsigned int seed) : Solver<Solution, Instance>(instance) {
		std::seed_seq ss { seed };

		threads_ = 1;

#ifdef _OPENMP
		threads_ = omp_get_max_threads();
#endif

		std::vector<std::seed_seq::result_type> seeds(threads_+1);

		ss.generate(std::begin(seeds), std::end(seeds));

		for (auto i : seeds) {
			mt_.emplace_back(i);
		}


		timer_.stop();
	}

	virtual ~GRASP() {
	}

	SolutionStatus operator()() override {
		if (!constructor_) {
			throw new UndefiniedOperator("Constructor");
		}
		timer_.resume();

		auto comparator = [&comp = comparator_](const std::unique_ptr<Solution> &lhs, const std::unique_ptr<Solution> &rhs) {
			return (*comp)(*lhs, *rhs);
		};

		containers::BestSet<std::unique_ptr<Solution>, decltype(comparator)> bestPool(
				bestPoolSize_, comparator);

		unsigned int currentIteration = 0;

#ifdef _OPENMP
#pragma omp parallel
		{
#endif
		bool stop = false;

		std::uniform_real_distribution<> alphaDistribution(0, 1);

		int currentThread = 0;
#ifdef _OPENMP
		currentThread = omp_get_thread_num();
#endif

		while (!stop) {
			int threadIterationt;
#ifdef _OPENMP
#pragma omp critical
			{
#endif
			threadIterationt = ++currentIteration;
#ifdef _OPENMP
			}
	#endif

			VLOG(1) << "Iterazione corrente: " << threadIterationt;

			double alpha = alphaDistribution(mt_[threads_]);
			std::unique_ptr<Solution> currentSolution = (*constructor_)(
					this->m_instance, mt_[currentThread], alpha);

			if (localSearch_) {
				(*localSearch_)(*currentSolution);
			}

#ifdef _OPENMP
#pragma omp critical
			{
#endif
				if (bestPool.size() == 0 || currentSolution->getCost() < bestPool[0]->getCost()) {
					incumbents_.emplace_back(currentIteration, static_cast<double>(timer_.elapsed().wall) / 1000000000LL, static_cast<double>(currentSolution->getCost()));
				}
			bestPool.add(std::move(currentSolution));
#ifdef _OPENMP
		}
#endif

#ifdef _OPENMP
#pragma omp critical
			{
#endif

				if (timeLimit_ > 0 && ((double) timer_.elapsed().wall / 1000000000LL) > timeLimit_) {
					LOG_IF(currentThread == 0, DEBUG) << "Esco perché time limit";
					stop = true;
				}
				else if (maxIterations_ > 0 && currentIteration > maxIterations_) {
					LOG_IF(currentThread == 0, DEBUG) << "Esco perché maxiterations";
					stop = true;
				}
				else if (target_ < std::numeric_limits<double>::infinity() && bestPool[0]->getCost() < target_) {
					LOG_IF(currentThread == 0, DEBUG) << "Esco perché target";
					stop = true;
				}
#ifdef _OPENMP
		}
#endif
		}
#ifdef _OPENMP
	}
#endif

		solution_ = bestPool.pop();
		timer_.stop();
		return SolutionStatus::kFeasible ;
	}

	GRASP &setComparator(std::unique_ptr<Comparator> &&comparator) {
		comparator_ = std::move(comparator);
		return *this;
	}

	template<typename ... Args>
	GRASP &setComparator(Args&& ... args) {
		comparator_.reset(new Comparator(std::forward<Args>(args)...));
		return *this;
	}

	GRASP &setConstructor(std::unique_ptr<Constructor> &&constructor) {
		constructor_ = std::move(constructor);
		return *this;
	}

	template<typename ... Args>
	GRASP &setConstructor(Args&& ... args) {
		constructor_.reset(new Constructor(std::forward<Args>(args)...));
		return *this;
	}

	GRASP &setLocalSearch(std::unique_ptr<LocalSearch> &&localSearch) {
		localSearch_ = std::move(localSearch);
		return *this;
	}

	template<typename ... Args>
	GRASP &setLocalSearch(Args&& ... args) {
		localSearch_.reset(new LocalSearch(std::forward<Args>(args)...));
		return *this;
	}

	GRASP &setPathRelink(std::unique_ptr<PathRelinking> &&pathRelink) {
		pathRelink_ = std::move(pathRelink);
		return *this;
	}

	template<typename ... Args>
	GRASP &setPathRelink(Args&& ... args) {
		pathRelink_.reset(new PathRelinking(std::forward<Args>(args)...));
		return *this;
	}

	void setBestPoolSize(
			typename containers::BestSet<Solution>::size_type size) {
		bestPoolSize_ = size;
	}

	std::unique_ptr<Solution> getSolution() override {
		return std::move(solution_);
	}

	GRASP &setTimeLimit(uint timeLimit) override {
		timeLimit_ = timeLimit;
		return *this;
	}

	GRASP &setTarget(double target) override {
		target_ = target;
		return *this;
	}

	GRASP &setMaxIterations(uint maxIterations) {
		maxIterations_ = maxIterations;
		return *this;
	}

	AlgorithmStatus getAlgorithmStatus() const override {
		return algorithmStatus_;
	}

	double getTime() const override {
		return static_cast<double>(timer_.elapsed().wall) / 1000000000LL;
	}

	const std::vector<std::tuple<uint, double, double>> &getIncumbents() const {
		return incumbents_;
	}


private:
	std::unique_ptr<Constructor> constructor_ { nullptr };
	std::unique_ptr<LocalSearch> localSearch_ { nullptr };
	std::unique_ptr<PathRelinking> pathRelink_ { nullptr };

	std::unique_ptr<Comparator> comparator_ { nullptr };

	typename containers::BestSet<Solution>::size_type bestPoolSize_ {
			1 };

	std::vector<std::mt19937_64> mt_;
	unsigned int threads_;

	std::unique_ptr<Solution> solution_ { nullptr };

	uint timeLimit_ { 0 };
	double target_ { std::numeric_limits<double>::infinity() };
	AlgorithmStatus algorithmStatus_ { AlgorithmStatus::kUnknown };
	uint maxIterations_ { 0 };
	std::vector<std::tuple<uint, double, double>> incumbents_ {};

	boost::timer::cpu_timer timer_;

	class UndefiniedOperator: public std::exception {
	public:
		UndefiniedOperator(const char *what) {
			std::sprintf(what_, "%s not definied", what);
		}
		const char* what() const noexcept {
			return what_;
		}

	private:
		char what_[100];
	};

};

} /* namespace metaheuristics */
} /* namespace dferone */

#endif /* GRASP_H_ */
