#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <simple_rwlock_test/clock.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/single_thread_tests.h>
#include <simple_rwlock_test/tests/two_thread_tests.h>
#include <simple_rwlock_test/tester.h>

namespace simple_rwlock_test {
    Tester::Tester() :
        tester_clock_(Clock()),
        tests_(std::vector<Test *>())
    {
        Clock::clk_latency_t start_time = tester_clock_.latency_from_start();
        std::cout << "Constructing tester at "
            << Clock::latency_to_string(start_time)
            << " from start" << std::endl;

        tests_.push_back(new TestSingleThreadInit(tester_clock_));
        tests_.push_back(new TestSingleThreadRead(tester_clock_));
        tests_.push_back(new TestSingleThreadWrite(tester_clock_));
        tests_.push_back(new TestSingleThreadReadWrite(tester_clock_));
        tests_.push_back(new TestSingleThreadWriteRead(tester_clock_));
        tests_.push_back(new TestTwoThreadReadOnceEach(tester_clock_));
        tests_.push_back(new TestTwoThreadReadWaitForOtherRead(tester_clock_));
        tests_.push_back(new TestTwoThreadReadWaitForOtherWrite(tester_clock_));
    }

    Tester::~Tester() {
        Clock::clk_latency_t end_time = tester_clock_.latency_from_start();
        std::cout << "Destructing tester at "
            << Clock::latency_to_string(end_time)
            << " from start" << std::endl;
        for (auto test : tests_) {
            delete test;
        }
    }

    int Tester::run_tests() {
        std::vector<std::string> failure_messages;
        std::vector<test_result_t> test_results;
        size_t max_name_length = 0;
        for (auto test : tests_) {
            std::cout << std::endl;
            Clock::clk_latency_t latency;
            std::string name = test->get_name();
            if (test->run_test(latency)) {
                failure_messages.push_back(name);
            } else {
                max_name_length = std::max(max_name_length, name.length());
                test_result_t result;
                result.test_name = name;
                result.test_time = latency;
                test_results.push_back(result);
            }
        }
        std::cout << std::endl;
        size_t num_failed = failure_messages.size();
        if (num_failed > 0) {
            std::cout << num_failed << " failed "
                << ((num_failed == 1) ? "test" : "tests") << ":" << std::endl;
            for (const auto &message : failure_messages) {
                std::cout << "\t" << message << std::endl;
            }
            std::cout << std::endl;
        } else {
            std::cout << "All tests passed" << std::endl << std::endl;
        }
        std::cout << "Summary of passing tests and run times:" << std::endl;
        for (const auto &result : test_results) {
            std::cout << std::setw(max_name_length)
                << std::setfill(' ') << result.test_name << ": ";
            std::cout << Clock::latency_to_string(result.test_time)
                << std::endl;
        }
        std::cout << std::endl;
        return 0;
    }
}
