#ifndef SRWLT_TESTER_H
#define SRWLT_TESTER_H

#include <vector>

#include <simple_rwlock_test/clock.h>
#include <simple_rwlock_test/test.h>

namespace simple_rwlock_test {
    class Tester {
    public:
        Tester();
        ~Tester();
        int run_tests();

    private:
        Clock tester_clock_;
        std::vector<Test *> tests_;
    };
}

#endif // SRWLT_TESTER_H
