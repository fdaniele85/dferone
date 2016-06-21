/*
 * Constructor.h
 *
 *  Created on: 20 giu 2016
 *      Author: daniele
 */

#ifndef IMPLEMENTATION_CONSTRUCTOR_H_
#define IMPLEMENTATION_CONSTRUCTOR_H_

#include <memory>

namespace dferone {
namespace metaheuristics {
namespace implementation {

template <typename Instance, typename Solution, typename Rnd>
class Constructor {
public:
	virtual ~Constructor() {};
	virtual std::unique_ptr<Solution> operator()(const Instance &instance, Rnd &mt, double alpha) = 0;
};

}
}
}


#endif /* IMPLEMENTATION_CONSTRUCTOR_H_ */
