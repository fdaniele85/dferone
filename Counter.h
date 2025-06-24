/*
 * Counter.h
 *
 *  Created on: 01 apr 2021
 *      Author: daniele
 */

#pragma once

#include <map>
#include <string>
#include <compare>

#ifdef DFERONE_THREAD_SAFE
#include <mutex>
#endif

namespace dferone::counters {

    /// \brief Global counter that can be used in different places in the code
    class Counter {
    public:
        /// Consrtuctor
        /// \param name Name of the counter
        explicit Counter(std::string name = "");

        virtual ~Counter() = default;

        /// Assigns val to the Counter
        Counter &operator=(double val);

        /// Conversion operator
        /// \return The value of the counter
        operator double() const;

        /// Post-increment operator
        double operator++(int);

        /// Pre-increment operator
        double operator++();

        /// Addition assignment
        /// \param val Value to add
        /// \return Result of the sum
        double operator+=(double val);

        /// Subtraction assignment
        /// \param val Value to subtract
        /// \return Result of the subtraction
        double operator-=(double val);

        /// Three-way comparison opertor
        auto operator<=>(const Counter &other) const {
#ifdef DFERONE_THREAD_SAFE
            std::lock_guard<std::mutex> _(mutex);
#endif
            return static_counters_[name_] <=> static_counters_[other.name_];
        }

        /// Equality opertor
        bool operator==(const Counter &other) const;

    private:
        // The globaal map of the counters
        static std::map<std::string, double> static_counters_;

#ifdef DFERONE_THREAD_SAFE
        // The mutex to enable parallelism
        static std::mutex mutex;
#endif

        // The name of the current counter
        std::string name_;
    };
} /* namespace dferone */
