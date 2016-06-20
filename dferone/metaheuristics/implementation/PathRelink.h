/*
 * PathRelink.h
 *
 *  Created on: 20 giu 2016
 *      Author: daniele
 */

#ifndef IMPLEMENTATION_PATHRELINK_H_
#define IMPLEMENTATION_PATHRELINK_H_

#include <memory>

namespace dferone {
namespace metaheuristics {
namespace implementation {

template <typename Solution>
class PathRelink {
public:
	virtual ~PathRelink() {}

	virtual std::unique_ptr<Solution> operator()(const Solution &lhs, const Solution &rhs) = 0;
};

}
}
}


#endif /* IMPLEMENTATION_PATHRELINK_H_ */
