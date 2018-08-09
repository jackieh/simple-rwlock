#include <simple_rwlock_test/test.h>

namespace simple_rwlock_test {
    // test_many_readers_one_writer: Have 8 reader threads and
    // one writer thread. Don't let the writer start writing
    // until after one of the readers has started reading.
    // Don't let any of the readers let go of the reader lock
    // until all of the readers have reported reading at least once.
    class TestManyReadersOneWriter : public Test {
    public:
        TestManyReadersOneWriter(Clock &tester_clock);
        int run_test_body();
    };
}
