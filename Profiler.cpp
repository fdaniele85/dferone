#include "Profiler.h"
#include "Counter.h"

namespace dferone {

#ifdef DFERONE_THREAD_SAFE
    constinit std::mutex Profiler::mutex = std::mutex();
#endif

    std::map<std::string, lemon::Timer> Profiler::static_timers_ = std::map<std::string, lemon::Timer>();

    void Profiler::resume(const std::string &name) {
        if (!static_timers_.contains(name)) {
            static_timers_[name] = lemon::Timer();
        } else {
            static_timers_[name].restart();
        }
        counters::Counter c(name);
        ++c;
    }

    void Profiler::stop(const std::string &name) { static_timers_[name].stop(); }

    void Profiler::print(std::ostream &os) {
        for (const auto &p : static_timers_) {
            counters::Counter c(p.first);
            os << "Timer " << p.first << ": " << p.second.realTime() << " total seconds, " << (p.second.realTime() / c) << " seconds per call\n";
        }
    }
} // namespace dferone