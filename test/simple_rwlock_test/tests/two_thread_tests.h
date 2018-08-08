#include <simple_rwlock_test/test.h>

namespace simple_rwlock_test {
    // test_two_thread_read_once_each: Have two threads, each which reads
    // from shared data once, confirms the correct value, and then ends.
    class TestTwoThreadReadOnceEach : public Test {
    public:
        TestTwoThreadReadOnceEach(Clock &tester_clock);
        int run_test_body();
    };

    // test_two_thread_wait_for_other_read: Have two threads, each
    // which repeatedly reads from shared data until the other
    // thread reports having read the shared data at least once.
    class TestTwoThreadReadWaitForOtherRead : public Test {
    public:
        TestTwoThreadReadWaitForOtherRead(Clock &tester_clock);
        int run_test_body();
    };
}
