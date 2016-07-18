/*
 * Heuristic.h
 *
 *  Created on: 19 giu 2016
 *      Author: daniele
 */

#ifndef SOLVER_H_
#define SOLVER_H_

#include <memory>

namespace dferone {
namespace metaheuristics {

enum class SolutionStatus { kFeasible, kOptimal, kUnfeasible, kUnknown, kRetrieved };
enum class AlgorithmStatus { kTarget, kNormal, kTimeLimit, kRunning, kUnknown };

template <typename Solution, typename Instance>
class Solver {
public:
	Solver(const Instance &instance) : m_instance(instance) {};
	virtual ~Solver() {};

	virtual SolutionStatus operator()() = 0;

	virtual std::unique_ptr<Solution> getSolution() = 0;
	virtual Solver &setTimeLimit(uint timeLimit) = 0;
	virtual Solver &setTarget(double target) = 0;
	virtual AlgorithmStatus getAlgorithmStatus() const = 0;

protected:
	const Instance &m_instance;
};

} /* namespace metaheuristics */
} /* namespace dferone */

#endif /* SOLVER_H_ */
