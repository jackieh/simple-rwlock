#include <iostream>

#include <simple_rwlock_test/clock.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/single_thread_tests.h>
#include <simple_rwlock_test/tester.h>

namespace simple_rwlock_test {
    Tester::Tester() :
        tester_clock_(Clock()),
        tests_(std::vector<Test *>())
    {
        Clock::clk_latency_t start_time = tester_clock_.latency_from_start();
        std::cout << "Constructing tester at "
            << Clock::latency_to_string(start_time)
            << " microseconds from start" << std::endl;

        tests_.push_back(new TestSingleThreadInit(
                "Single thread init/uninit", tester_clock_));
        tests_.push_back(new TestSingleThreadRead(
                "Single thread read", tester_clock_));
        tests_.push_back(new TestSingleThreadWrite(
                "Single thread write", tester_clock_));
    }

    Tester::~Tester() {
        Clock::clk_latency_t end_time = tester_clock_.latency_from_start();
        std::cout << "Destructing tester at "
            << Clock::latency_to_string(end_time)
            << " microseconds from start" << std::endl;
        for (auto test : tests_) {
            delete test;
        }
    }

    int Tester::run_tests() {
        std::vector<std::string> failure_messages;
        for (auto test : tests_) {
            std::cout << std::endl;
            if (test->run_test()) {
                failure_messages.push_back(test->get_name());
            }
        }
        std::cout << std::endl;
        if (failure_messages.size() > 0) {
            std::cout << failure_messages.size()
                << " failed tests:" << std::endl;
            for (const auto &message : failure_messages) {
                std::cout << "\t" << message << std::endl;
            }
        } else {
            std::cout << "All tests passed" << std::endl;
        }
        std::cout << std::endl;
        return 0;
    }
}
