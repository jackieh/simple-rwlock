#include <iostream>
#include <thread>

#include <simple_rwlock.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/single_thread_tests.h>

using namespace simple_rwlock;

// Have a separate sub-namespace for each test in order to
// prevent concerns about duplicated function names.
namespace simple_rwlock_test {
    // Function run by the thread in the TestSingleThreadInit test class.
    namespace test_single_thread_init {
        // Initialize the rwlock object and then uninitialize the object.
        void init_rwlock(rwlock_t *rwlock) {
            rwlock_init(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadInit::TestSingleThreadInit(Clock &tester_clock) :
        Test("single_thread_init", tester_clock)
    { }
    int TestSingleThreadInit::run_test_body() {
        using namespace test_single_thread_init;
        rwlock_t shared_rwlock;
        std::thread only_thread(init_rwlock, &shared_rwlock);
        only_thread.join();
        return 0;
    }

    // Function run by the thread in the TestSingleThreadRead test class.
    namespace test_single_thread_read {
        // Read from the data once.
        void rlock(rwlock_t *rwlock, unsigned int *data, bool *rlock_pass) {
            rwlock_init(rwlock);
            rwlock_lock_rd(rwlock);
#ifdef DEBUG
            std::cout << "(Thread " << std::this_thread::get_id()
                << ")\trlock: data is 0x" << std::hex << *data << std::endl;
#endif
            *rlock_pass &= (*data == 0xdeadbeef);
            rwlock_unlock_rd(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadRead::TestSingleThreadRead(Clock &tester_clock) :
        Test("single_thread_read", tester_clock)
    { }
    int TestSingleThreadRead::run_test_body() {
        using namespace test_single_thread_read;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool rlock_pass = true;
        std::thread only_thread(rlock, &shared_rwlock, &data, &rlock_pass);
        only_thread.join();
        return (rlock_pass ? 0 : 1);
    }

    // Function run by the thread in the TestSingleThreadWrite test class.
    namespace test_single_thread_write {
        // Acquire a write lock, then write to and read from the data.
        void wlock(rwlock_t *rwlock, unsigned int *data, bool *wlock_pass) {
            rwlock_init(rwlock);
            rwlock_lock_wr(rwlock);
#ifdef DEBUG
            std::cout << "(Thread " << std::this_thread::get_id()
                << ")\twlock: data is 0x" << std::hex << *data << std::endl;
#endif
            *wlock_pass &= (*data == 0xdeadbeef);
            *data = 0xfeedcafe;
            *wlock_pass &= (*data == 0xfeedcafe);
            rwlock_unlock_wr(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadWrite::TestSingleThreadWrite(Clock &tester_clock) :
        Test("single_thread_write", tester_clock)
    { }
    int TestSingleThreadWrite::run_test_body() {
        using namespace test_single_thread_write;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool wlock_pass = true;
        std::thread only_thread(wlock, &shared_rwlock, &data, &wlock_pass);
        only_thread.join();
        return (wlock_pass ? 0 : 1);
    }

    // Function run by the thread in the TestSingleThreadReadWrite test class.
    namespace test_single_thread_read_write {
        // Acquire a read lock, read from the data, and release the
        // read lock. Then acquire a write lock, write to and read
        // from the data, and release the write lock.
        void rw(rwlock_t *rwlock, unsigned int *data, bool *rw_pass) {
            rwlock_init(rwlock);
            rwlock_lock_rd(rwlock);
#ifdef DEBUG
            std::cout << "(Thread " << std::this_thread::get_id()
                << ")\trw: data is 0x" << std::hex << *data << std::endl;
#endif
            *rw_pass &= (*data == 0xdeadbeef);
            rwlock_unlock_rd(rwlock);
            rwlock_lock_wr(rwlock);
#ifdef DEBUG
            std::cout << "(Thread " << std::this_thread::get_id()
                << ")\trw: data is 0x" << std::hex << *data << std::endl;
#endif
            *rw_pass &= (*data == 0xdeadbeef);
            *data = 0xfeedcafe;
            *rw_pass &= (*data == 0xfeedcafe);
            rwlock_unlock_wr(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadReadWrite::TestSingleThreadReadWrite(Clock &tester_clock) :
        Test("single_thread_read_write", tester_clock)
    { }
    int TestSingleThreadReadWrite::run_test_body() {
        using namespace test_single_thread_read_write;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool rw_pass = true;
        std::thread only_thread(rw, &shared_rwlock, &data, &rw_pass);
        only_thread.join();
        return (rw_pass ? 0 : 1);
    }

    // Function run by the thread in the TestSingleThreadWriteRead test class.
    namespace test_single_thread_write_read {
        // Acquire a write lock, write to and read from the data,
        // and release the write lock. Then acquire a read lock,
        // read from the data, and release the read lock.
        void wr(rwlock_t *rwlock, unsigned int *data, bool *wr_pass) {
            rwlock_init(rwlock);
            rwlock_lock_wr(rwlock);
#ifdef DEBUG
            std::cout << "(Thread " << std::this_thread::get_id()
                << ")\twr: data is 0x" << std::hex << *data << std::endl;
#endif
            *wr_pass &= (*data == 0xdeadbeef);
            *data = 0xfeedcafe;
            *wr_pass &= (*data == 0xfeedcafe);
            rwlock_unlock_wr(rwlock);
            rwlock_lock_rd(rwlock);
#ifdef DEBUG
            std::cout << "(Thread " << std::this_thread::get_id()
                << ")\twr: data is 0x" << std::hex << *data << std::endl;
#endif
            *wr_pass &= (*data == 0xfeedcafe);
            rwlock_unlock_rd(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadWriteRead::TestSingleThreadWriteRead(Clock &tester_clock) :
        Test("single_thread_write_read", tester_clock)
    { }
    int TestSingleThreadWriteRead::run_test_body() {
        using namespace test_single_thread_write_read;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool wr_pass = true;
        std::thread only_thread(wr, &shared_rwlock, &data, &wr_pass);
        only_thread.join();
        return (wr_pass ? 0 : 1);
    }
}
