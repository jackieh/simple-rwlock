#include <iostream>
#include <thread>

#include <simple_rwlock.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/single_thread_tests.h>

using namespace simple_rwlock;

// Have a separate sub-namespace for each test in order to
// prevent concerns about duplicated function names.
namespace simple_rwlock_test {
    namespace test_single_thread_init {
        void init_rwlock(rwlock_t *rwlock) {
            rwlock_init(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadInit::TestSingleThreadInit(
        std::string test_name, Clock &tester_clock) :
        Test(test_name, tester_clock)
    { }
    int TestSingleThreadInit::run_test_body() {
        using namespace test_single_thread_init;
        rwlock_t shared_rwlock;
        std::thread only_thread(init_rwlock, &shared_rwlock);
        only_thread.join();
        return 0;
    }

    namespace test_single_thread_read {
        void read_lock(rwlock_t *rwlock, unsigned int *data,
                       bool *read_lock_pass)
        {
            rwlock_init(rwlock);
            rwlock_lock_rd(rwlock);
            std::cout << "Data is 0x" << std::hex << *data << std::endl;
            *read_lock_pass &= *data == 0xdeadbeef;
            rwlock_unlock_rd(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadRead::TestSingleThreadRead(
        std::string test_name, Clock &tester_clock) :
        Test(test_name, tester_clock)
    { }
    int TestSingleThreadRead::run_test_body() {
        using namespace test_single_thread_read;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool read_lock_pass = true;
        std::thread only_thread(
            read_lock, &shared_rwlock, &data, &read_lock_pass);
        only_thread.join();
        return 0;
    }

    namespace test_single_thread_write {
        void write_lock(rwlock_t *rwlock, unsigned int *data,
                        bool *write_lock_pass)
        {
            rwlock_init(rwlock);
            rwlock_lock_wr(rwlock);
            std::cout << "Data is 0x" << std::hex << *data << std::endl;
            *write_lock_pass &= *data == 0xdeadbeef;
            *data = 0xfeedcafe;
            *write_lock_pass &= *data == 0xfeedcafe;
            rwlock_unlock_wr(rwlock);
            rwlock_uninit(rwlock);
        }
    }
    TestSingleThreadWrite::TestSingleThreadWrite(
        std::string test_name, Clock &tester_clock) :
        Test(test_name, tester_clock)
    { }
    int TestSingleThreadWrite::run_test_body() {
        using namespace test_single_thread_write;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool write_lock_pass = true;
        std::thread only_thread(
            write_lock, &shared_rwlock, &data, &write_lock_pass);
        only_thread.join();
        return 0;
    }
}
