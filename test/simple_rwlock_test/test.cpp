#include <iomanip>
#include <iostream>
#include <string>

#include <simple_rwlock.h>
#include <simple_rwlock_test/clock.h>
#include <simple_rwlock_test/test.h>

using namespace simple_rwlock;

namespace simple_rwlock_test {
    Test::Test(std::string test_name, Clock &tester_clock) :
        test_name_(test_name),
        tester_clock_(tester_clock)
    { }

    Test::Test(Test &other) :
        test_name_(other.test_name_),
        tester_clock_(other.tester_clock_)
    { }

    Test &Test::operator=(const Test &other) {
        test_name_ = other.test_name_;
        tester_clock_ = other.tester_clock_;
        return *this;
    }

    Test::~Test() { }

    std::string Test::get_name() const {
        return test_name_;
    }

    int Test::run_test(Clock::clk_latency_t &test_latency) {
        begin_test();
        int result = run_test_body();
        end_test(test_latency);
        return result;
    }

    void Test::begin_test() {
        tester_clock_.update_checkpoint();
        Clock::clk_latency_t start_time =
            tester_clock_.latency_to_checkpoint();
        std::cout << "Begin " << std::quoted(test_name_)
            << " test at " << Clock::latency_to_string(start_time)
            << " from start of testing" << std::endl;
#ifdef DEBUG
        std::cout << "---" << std::endl;
#endif
    }

    void Test::end_test(Clock::clk_latency_t &test_latency) {
        // Report how long the test took to run.
        tester_clock_.update_checkpoint();
        Clock::clk_latency_t end_time =
            tester_clock_.latency_to_checkpoint();
        Clock::clk_latency_t test_time =
            tester_clock_.latency_between_checkpoints();
        test_latency = test_time;
#ifdef DEBUG
        std::cout << "---" << std::endl;
#endif
        std::cout << "End " << std::quoted(test_name_)
            << " test at " << Clock::latency_to_string(end_time)
            << " from start (" << Clock::latency_to_string(test_time)
            << " from beginning of test)" << std::endl;
    }
}
