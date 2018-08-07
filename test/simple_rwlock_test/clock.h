#ifndef SRWLT_CLOCK_H
#define SRWLT_CLOCK_H

#include <chrono>
#include <string>

namespace simple_rwlock_test {
    // Class for wrapping around some of the functionality of std::chrono.
    class Clock {
    public:
        typedef
            std::chrono::duration<long int, std::ratio<1, 1000000> >::rep
            clk_latency_t;

        Clock();
        ~Clock();

        // Return count of microseconds from start to now.
        clk_latency_t latency_from_start();

        // Return number of microseconds from current checkpoint to now.
        clk_latency_t latency_from_checkpoint();

        // Return number of microseconds from start to current checkpoint.
        clk_latency_t latency_to_checkpoint();

        // Return number of microseconds from previous checkpoint to
        // current checkpoint.
        clk_latency_t latency_between_checkpoints();

        // Record a new checkpoint.
        void update_checkpoint();

        // Return number of microseconds as a
        // formatted string with a fixed precision.
        static std::string latency_to_string(clk_latency_t latency);

    private:
        typedef std::chrono::microseconds
            clk_microseconds_t;
        typedef
            std::chrono::time_point<
                std::chrono::system_clock,
                std::chrono::duration<long int, std::ratio<1, 1000000000> > >
            clk_time_t;

        static inline clk_time_t get_now();
        static inline clk_latency_t get_diff(clk_time_t time_earlier,
                                             clk_time_t time_later);

        clk_time_t start_time_;
        clk_time_t current_checkpoint_;
        clk_time_t previous_checkpoint_;
    };
}

#endif // SRWLT_CLOCK_H
