/*
 * Heuristic.h
 *
 *  Created on: 19 giu 2016
 *      Author: daniele
 */

#ifndef HEURISTIC_H_
#define HEURISTIC_H_

#include <memory>

namespace dferone {
namespace metaheuristics {

template <typename Solution, typename Instance>
class Heuristic {
public:
	virtual ~Heuristic() {};

	virtual std::unique_ptr<Solution> operator()(const Instance &instance) = 0;
};

} /* namespace metaheuristics */
} /* namespace dferone */

#endif /* HEURISTIC_H_ */
