#ifndef SRWLT_TEST_H
#define SRWLT_TEST_H

#include <string>

#include <simple_rwlock.h>
#include <simple_rwlock_test/clock.h>

using namespace simple_rwlock;

// Base class for tests. Each test is stored as an object
// in the tester. Specific tests are defined as subclasses
// of this class in the simple_rwlock_test/tests/ directory.
namespace simple_rwlock_test {
    class Test {
    public:
        Test(std::string test_name, Clock &tester_clock);
        Test(Test &other);
        Test &operator=(const Test &other);
        virtual ~Test();
        std::string get_name() const;
        int run_test(Clock::clk_latency_t &test_latency);

    protected:
        virtual int run_test_body() = 0;
        void begin_test();
        void end_test(Clock::clk_latency_t &test_latency);

        std::string test_name_;
        Clock &tester_clock_;
    };
}

#endif // SRWLT_TEST_H
