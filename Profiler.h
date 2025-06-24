/*
 * Profiler.h
 *
 *  Created on: 24 jun 2025
 *      Author: daniele
 */

#pragma once

#include <map>
#include <string>
#include <lemon/time_measure.h>

#ifdef DFERONE_THREAD_SAFE
#include <mutex>
#endif

namespace dferone {

    /// \brief Global counter that can be used in different places in the code
    class Profiler {
    public:
        virtual ~Profiler() = default;

        static void resume(const std::string &name);

        static void stop(const std::string &name);

        static void print(std::ostream &os = std::cout);

    private:
        // The global map of the timers
        static std::map<std::string, lemon::Timer> static_timers_;

#ifdef DFERONE_THREAD_SAFE
        // The mutex to enable parallelism
        static std::mutex mutex;
#endif

        // The name of the current counter
        std::string name_;

        explicit Profiler() = default;
    };
} /* namespace dferone */
