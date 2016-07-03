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


#include "Heuristic.h"
#include "implementation/PathRelink.h"

#include "../containers/BestSet.h"

namespace dferone {
namespace metaheuristics {

template<typename Instance, typename Solution, typename Constructor,
		typename LocalSearch, typename StopCriterion,
		typename PathRelinking = implementation::PathRelink<Solution>,
		typename Comparator = std::less<Solution>>
class GRASP: public Heuristic<Solution, Instance> {
public:

	GRASP(unsigned int seed) {
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


	}

	virtual ~GRASP() {
	}

	virtual std::unique_ptr<Solution> operator()(const Instance &instance) {
		if (!constructor_) {
			throw new UndefiniedOperator("Constructor");
		}

		if (!stopCriterion_) {
			throw new UndefiniedOperator("StopCriterion");
		}

		auto comparator = [&comp = comparator_](const std::unique_ptr<Solution> &lhs, const std::unique_ptr<Solution> &rhs) {
			return (*comp)(*lhs, *rhs);
		};

		containers::BestSet<std::unique_ptr<Solution>, decltype(comparator)> bestPool(
				bestPoolSize_, comparator);

		unsigned int currentIteration = 0;

		boost::timer::cpu_timer timer;

#ifdef _OPENMP
#pragma omp parallel
		{
#endif
		bool stop = false;

		std::uniform_real_distribution<> alphaDistribution(0, 1);

		int currentThread = 0;
#ifdef _OPENMP
		currentThread = omp_get_team_num();
#endif

		while (!stop) {
#ifdef _OPENMP
#pragma omp critical
			{
#endif
			++currentIteration;
#ifdef _OPENMP
			}
	#endif

			VLOG(1) << "Iterazione corrente: " << currentIteration;

			double alpha = alphaDistribution(mt_[threads_]);
			std::unique_ptr<Solution> currentSolution = (*constructor_)(
					instance, mt_[currentThread], alpha);

			if (localSearch_) {
				(*localSearch_)(*currentSolution);
			}

#ifdef _OPENMP
#pragma omp critical
			{
#endif
			bestPool.add(std::move(currentSolution));
#ifdef _OPENMP
		}
#endif

#ifdef _OPENMP
#pragma omp critical
			{
#endif

				stop = (*stopCriterion_)(currentIteration,
					((double) timer.elapsed().wall / 1000000000LL),
					bestPool[0]->getCost());
#ifdef _OPENMP
		}
#endif
		}
#ifdef _OPENMP
	}
#endif

		return bestPool.pop();
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

	GRASP &setStopCriterion(std::unique_ptr<StopCriterion> &&stopCriterion) {
		stopCriterion_ = std::move(stopCriterion);
		return *this;
	}

	template<typename ... Args>
	GRASP &setStopCriterion(Args&& ... args) {
		stopCriterion_.reset(new StopCriterion(std::forward<Args>(args)...));
		return *this;
	}

	void setBestPoolSize(
			typename containers::BestSet<Solution>::size_type size) {
		bestPoolSize_ = size;
	}

private:
	std::unique_ptr<Constructor> constructor_ { nullptr };
	std::unique_ptr<LocalSearch> localSearch_ { nullptr };
	std::unique_ptr<StopCriterion> stopCriterion_ { nullptr };
	std::unique_ptr<PathRelinking> pathRelink_ { nullptr };

	std::unique_ptr<Comparator> comparator_ { nullptr };

	typename containers::BestSet<Solution>::size_type bestPoolSize_ {
			1 };

	std::vector<std::mt19937_64> mt_;
	unsigned int threads_;

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
