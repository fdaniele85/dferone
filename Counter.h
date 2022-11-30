/*
 * Counter.h
 *
 *  Created on: 01 apr 2021
 *      Author: daniele
 */

#pragma once

#include <boost/timer/timer.hpp>
#include <map>
#include <string>

#ifdef DFERONE_THREAD_SAFE
#include <mutex>
#endif

namespace dferone::counters {

    /// \brief Global counter that can be used in different places in the code
    class Counter {
    public:
        explicit Counter(std::string name = "");

        virtual ~Counter() = default;

        Counter &operator=(double val);

        operator double() const;

        double operator++(int);

        double operator++();

        double operator+=(double val);

        double operator-=(double val);

    private:
        static Counter *singleton;
        std::map<std::string, double> counters_;
        static std::map<std::string, double> static_counters_;
#ifdef DFERONE_THREAD_SAFE
        static std::mutex mutex;
#endif
        std::string name_;
    };
} /* namespace dferone */
