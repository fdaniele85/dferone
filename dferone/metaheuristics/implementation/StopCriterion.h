/*
 * StopCriterion.h
 *
 *  Created on: 20 giu 2016
 *      Author: daniele
 */

#ifndef IMPLEMENTATION_STOPCRITERION_H_
#define IMPLEMENTATION_STOPCRITERION_H_

namespace dferone {
namespace metaheuristics {
namespace implementation {

class StopCriterion {
public:
	virtual ~StopCriterion() {};
	virtual bool operator()(int currentIteration, double timeElapsed, double incumbentCost) = 0;
};

}
}
}

#endif /* IMPLEMENTATION_STOPCRITERION_H_ */
