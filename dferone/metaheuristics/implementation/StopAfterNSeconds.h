/*
 * StopAfterNSeconds.h
 *
 *  Created on: 20 giu 2016
 *      Author: daniele
 */

#ifndef IMPLEMENTATION_STOPAFTERNSECONDS_H_
#define IMPLEMENTATION_STOPAFTERNSECONDS_H_

#include "StopCriterion.h"

namespace dferone {
namespace metaheuristics {
namespace implementation {

class StopAfetrNSeconds : public StopCriterion {
public:
	StopAfetrNSeconds(unsigned int seconds) : seconds_(seconds) {}
	bool operator()(int currentIteration, double timeElapsed, double incumbentCost) override {
		return timeElapsed > seconds_;
	}

private:
	unsigned int seconds_;
};

}
}
}

#endif /* IMPLEMENTATION_STOPAFTERNSECONDS_H_ */
