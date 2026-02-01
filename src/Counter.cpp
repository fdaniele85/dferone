/*
 * Counter.cpp
 *
 *  Created on: 01 apr 2021
 *      Author: daniele
 */

#include "../include/dferone/Counter.h"

namespace dferone::counters {

#ifdef DFERONE_THREAD_SAFE
constinit std::mutex Counter::mutex = std::mutex();
#endif

std::map<std::string, double> Counter::static_counters_ = std::map<std::string, double>();

    Counter::Counter(std::string name) : name_(name) {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        if (!Counter::static_counters_.contains(name)) {
            Counter::static_counters_[name] = 0.0;
        }
    }


    Counter &Counter::operator =(double val) {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        static_counters_[name_] = val;
        return *this;
    }

    Counter::operator double() const {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        return static_counters_[name_];
    }

    double Counter::operator++(int) {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        double val = static_counters_[name_];
        ++static_counters_[name_];
        return val;
    }

    double Counter::operator++() {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        return ++static_counters_[name_];
    }

    double Counter::operator+=(double val) {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        static_counters_[name_] += val;
        return static_counters_[name_];
    }

    double Counter::operator-=(double val) {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif
        static_counters_[name_] -= val;
        return static_counters_[name_];
    }

    bool Counter::operator==(const Counter &other) const {
#ifdef DFERONE_THREAD_SAFE
        std::lock_guard<std::mutex> lock(mutex);
#endif

        if (name_ == other.name_) {
            return true;
        }

        return static_counters_[name_] == static_counters_[other.name_];
    }
}