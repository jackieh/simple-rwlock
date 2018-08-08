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

    int Test::run_test() {
        begin_test();
        int result = run_test_body();
        end_test();
        return result;
    }

    void Test::begin_test() {
        tester_clock_.update_checkpoint();
        Clock::clk_latency_t start_time =
            tester_clock_.latency_to_checkpoint();
        std::cout << "Begin " << std::quoted(test_name_)
            << " test at " << Clock::latency_to_string(start_time)
            << " microseconds from start of testing" << std::endl;
        std::cout << "---" << std::endl;
    }

    void Test::end_test() {
        // Report how long the test took to run.
        tester_clock_.update_checkpoint();
        Clock::clk_latency_t end_time =
            tester_clock_.latency_to_checkpoint();
        Clock::clk_latency_t test_time =
            tester_clock_.latency_between_checkpoints();
        std::cout << "---" << std::endl;
        std::cout << "End " << std::quoted(test_name_)
            << " test at " << Clock::latency_to_string(end_time)
            << " microseconds from start ("
            << Clock::latency_to_string(test_time)
            << " microseconds from beginning of test)" << std::endl;
    }
}
