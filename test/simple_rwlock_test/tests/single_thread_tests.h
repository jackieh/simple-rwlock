#include <simple_rwlock_test/test.h>

namespace simple_rwlock_test {
    class TestSingleThreadInit : public Test {
    public:
        TestSingleThreadInit(std::string test_name, Clock &tester_clock);
        int run_test_body();
    };

    class TestSingleThreadRead : public Test {
    public:
        TestSingleThreadRead(std::string test_name, Clock &tester_clock);
        int run_test_body();
    };

    class TestSingleThreadWrite : public Test {
    public:
        TestSingleThreadWrite(std::string test_name, Clock &tester_clock);
        int run_test_body();
    };
}
