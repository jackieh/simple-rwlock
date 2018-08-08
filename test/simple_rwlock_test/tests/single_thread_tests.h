#include <simple_rwlock_test/test.h>

namespace simple_rwlock_test {
    // test_single_thread_init: One thread initializes
    // and uninitializes a rwlock object.
    class TestSingleThreadInit : public Test {
    public:
        TestSingleThreadInit(Clock &tester_clock);
        int run_test_body();
    };

    // test_single_thread_read: One thread acquires a read
    // lock, reads from data, and releases the read lock.
    class TestSingleThreadRead : public Test {
    public:
        TestSingleThreadRead(Clock &tester_clock);
        int run_test_body();
    };

    // test_single_thread_write: One thread acquires a write lock,
    // writes to and reads from data, and releases the write lock.
    class TestSingleThreadWrite : public Test {
    public:
        TestSingleThreadWrite(Clock &tester_clock);
        int run_test_body();
    };

    // test_single_thread_read_write: One thread acquires a read
    // lock, reads from the data, and releases the read lock.
    // Then the thread acquires a write lock, writes to and
    // reads from the data, and releases the write lock.
    class TestSingleThreadReadWrite : public Test {
    public:
        TestSingleThreadReadWrite(Clock &tester_clock);
        int run_test_body();
    };

    // test_single_thread_write_read: One thread acquires a write
    // lock, writes to and reads from the data, and releases the
    // write lock. Then the thread acquires a read lock, reads
    // from the data, and releases the read lock.
    class TestSingleThreadWriteRead : public Test {
    public:
        TestSingleThreadWriteRead(Clock &tester_clock);
        int run_test_body();
    };
}
