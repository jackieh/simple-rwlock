#include <chrono>
#include <string>
#include <sstream>

#include <simple_rwlock_test/clock.h>

namespace simple_rwlock_test {
    Clock::Clock() :
        start_time_(get_now()),
        current_checkpoint_(get_now()),
        previous_checkpoint_(get_now())
    { }

    Clock::~Clock() { }

    Clock::clk_latency_t Clock::latency_from_start() {
        return get_diff(start_time_, get_now());
    }

    Clock::clk_latency_t Clock::latency_from_checkpoint() {
        return get_diff(current_checkpoint_, get_now());
    }

    Clock::clk_latency_t Clock::latency_to_checkpoint() {
        return get_diff(start_time_, current_checkpoint_);
    }

    Clock::clk_latency_t Clock::latency_between_checkpoints() {
        return get_diff(previous_checkpoint_, current_checkpoint_);
    }

    std::string Clock::latency_to_string(clk_latency_t latency) {
        std::stringstream repr_stream;
        repr_stream.precision(10);
        repr_stream << latency << " microseconds";
        return repr_stream.str();
    }

    void Clock::update_checkpoint() {
        previous_checkpoint_ = current_checkpoint_;
        current_checkpoint_ = Clock::get_now();
    }

    Clock::clk_time_t Clock::get_now() {
        return std::chrono::high_resolution_clock::now();
    }

    Clock::clk_latency_t Clock::get_diff(
        clk_time_t time_earlier, clk_time_t time_later)
    {
        auto delta_time = time_later - time_earlier;
        auto microseconds_duration =
            std::chrono::duration_cast<clk_microseconds_t>(delta_time);
        return microseconds_duration.count();
    }
}
