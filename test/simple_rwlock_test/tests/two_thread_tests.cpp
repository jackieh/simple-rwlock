#include <mutex>
#include <thread>

#include <simple_rwlock.h>
#include <simple_rwlock_debug_helpers.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/test_common.h>
#include <simple_rwlock_test/tests/two_thread_tests.h>

namespace simple_rwlock_test {
    using namespace simple_rwlock;
    using namespace test_common;

    // Functions run by threads in the TestTwoThreadReadOnceEach test class.
    namespace test_two_thread_read_once_each {
        // Read once from shared data and confirm it is the correct value.
        void rlock1(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock1_pass)  // Not shared
        {
            TEST_DLOG_THREAD_LAUNCH("rlock1");
            unsigned int data_value = 0;
            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }
            TEST_DLOG_VAR_VALUE_HEX("rlock1", "data", data_value);
            *rlock1_pass &= (data_value == 0xdeadbeef);
        }

        // Read once from shared data and confirm it is the correct value.
        void rlock2(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock2_pass)  // Not shared
        {
            TEST_DLOG_THREAD_LAUNCH("rlock2");
            unsigned int data_value = 0;
            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }
            TEST_DLOG_VAR_VALUE_HEX("rlock2", "data", data_value);
            *rlock2_pass &= (data_value == 0xdeadbeef);
        }
    }
    TestTwoThreadReadOnceEach::TestTwoThreadReadOnceEach(Clock &tester_clock) :
        Test("two_thread_read_once_each", tester_clock)
    { }
    int TestTwoThreadReadOnceEach::run_test_body() {
        using namespace test_two_thread_read_once_each;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool rlock1_pass = true;
        bool rlock2_pass = true;
        rwlock_init(&shared_rwlock);
        std::thread thread1(rlock1, &shared_rwlock, &data, &rlock1_pass);
        std::thread thread2(rlock2, &shared_rwlock, &data, &rlock2_pass);
        thread1.join();
        thread2.join();
        rwlock_uninit(&shared_rwlock);
        return (rlock1_pass && rlock2_pass && (data == 0xdeadbeef)) ? 0 : 1;
    }

    // Have two threads, each which repeatedly reads from shared data until
    // the other thread reports having read the shared data at least once.
    namespace test_two_thread_wait_for_other_read {
        // Repeatedly read from shared data until rlock2
        // reports having read the data at least once.
        void rlock1(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock_pass,   // Shared
                    bool *rlock1_read,  // Shared
                    bool *rlock2_read)  // Shared
        {
            TEST_DLOG_THREAD_LAUNCH("rlock1");
            unsigned int data_value = 0;
            bool rlock2_read_value = false;
            while (!rlock2_read_value) {
                data_value = 0;

                { // Critical section: read from data and rlock2_read.
                    rwlock_lock_rd(rwlock);
                    data_value = *data;
                    rlock2_read_value = *rlock2_read;
                    rwlock_unlock_rd(rwlock);
                }

                { // Critical section: write to rlock1_read and rlock_pass.
                    rwlock_lock_wr(rwlock);
                    *rlock1_read = true;
                    *rlock_pass &= (data_value == 0xdeadbeef);
                    rwlock_unlock_wr(rwlock);
                }

                TEST_DLOG_VAR_VALUE_HEX("rlock1", "data", data_value);
                std::this_thread::yield();
            } // End while loop
            data_value = 0;

            { // Critical section: read from data
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

            TEST_DLOG_VAR_VALUE_HEX("end of rlock1", "data", data_value);
        }

        // Repeatedly read from shared data until rlock1
        // reports having read the data at least once.
        void rlock2(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock_pass,   // Shared
                    bool *rlock1_read,  // Shared
                    bool *rlock2_read)  // Shared
        {
            TEST_DLOG_THREAD_LAUNCH("rlock2");
            unsigned int data_value = 0;
            bool rlock1_read_value = false;
            while (!rlock1_read_value) {
                data_value = 0;

                { // Critical section: read from data and rlock1_read.
                    rwlock_lock_rd(rwlock);
                    data_value = *data;
                    rlock1_read_value = *rlock1_read;
                    rwlock_unlock_rd(rwlock);
                }

                { // Critical section: write to rlock2_read and rlock_pass.
                    rwlock_lock_wr(rwlock);
                    *rlock2_read = true;
                    *rlock_pass &= (data_value == 0xdeadbeef);
                    rwlock_unlock_wr(rwlock);
                }

                TEST_DLOG_VAR_VALUE_HEX("rlock2", "data", data_value);
                std::this_thread::yield();
            } // End while loop
            data_value = 0;

            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

            TEST_DLOG_VAR_VALUE_HEX("end of rlock2", "data", data_value);
        }
    }
    TestTwoThreadReadWaitForOtherRead::TestTwoThreadReadWaitForOtherRead(
        Clock &tester_clock) :
        Test("test_two_thread_wait_for_other_read", tester_clock)
    { }
    int TestTwoThreadReadWaitForOtherRead::run_test_body() {
        using namespace test_two_thread_wait_for_other_read;
        rwlock_t shared_rwlock;
        unsigned int data = 0xdeadbeef;
        bool rlock_pass = true;
        bool rlock1_read = false;
        bool rlock2_read = false;
        rwlock_init(&shared_rwlock);
        std::thread thread1(rlock1, &shared_rwlock, &data,
                            &rlock_pass, &rlock1_read, &rlock2_read);
        std::thread thread2(rlock2, &shared_rwlock, &data,
                            &rlock_pass, &rlock1_read, &rlock2_read);
        thread1.join();
        thread2.join();
        rwlock_uninit(&shared_rwlock);
        rlock_pass &= (data == 0xdeadbeef);
        return (rlock_pass ? 0 : 1);
    }

    // Have two threads, each which repeatedly writes to shared data until the
    // other thread reports having written to the shared data at least once.
    namespace test_two_thread_wait_for_other_write {
        // Repeatedly write to shared data until wlock2
        // reports having written the data at least once.
        void wlock1(rwlock_t *rwlock,   // Shared
                    int *data,          // Shared
                    bool *wlock_pass,   // Shared
                    bool *wlock1_wrote, // Shared
                    bool *wlock2_wrote) // Shared
        {
            TEST_DLOG_THREAD_LAUNCH("wlock1");
            unsigned int data_value = 0;
            bool wlock2_wrote_value = false;

            while (!wlock2_wrote_value) {
                data_value = 0;

                { // Critical section: read from data and wlock2_wrote.
                    rwlock_lock_rd(rwlock);
                    wlock2_wrote_value = *wlock2_wrote;
                    data_value = *data;
                    rwlock_unlock_rd(rwlock);
                }

                { // Critical section: write to data, wlock1_wrote, wlock_pass.
                    rwlock_lock_wr(rwlock);
                    *data = (*data > 1000) ? 1 : (*data + 1);
                    *wlock1_wrote = true;
                    *wlock_pass &= (data_value > 0);
                    rwlock_unlock_wr(rwlock);
                }

                TEST_DLOG_VAR_VALUE_HEX("wlock1", "data", data_value);
                std::this_thread::yield();
            } // End while loop
            data_value = 0;

            { // Critical section: read from data
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

            TEST_DLOG_VAR_VALUE_HEX("end of wlock1", "data", data_value);
        }

        // Repeatedly write to shared data until wlock1
        // reports having written to the data at least once.
        void wlock2(rwlock_t *rwlock,   // Shared
                    int *data,          // Shared
                    bool *wlock_pass,   // Shared
                    bool *wlock1_wrote, // Shared
                    bool *wlock2_wrote) // Shared
        {
            TEST_DLOG_THREAD_LAUNCH("wlock2");
            unsigned int data_value = 0;
            bool wlock1_wrote_value = false;

            while (!wlock1_wrote_value) {
                data_value = 0;

                { // Critical section: read from data and wlock1_wrote.
                    rwlock_lock_rd(rwlock);
                    data_value = *data;
                    wlock1_wrote_value = *wlock1_wrote;
                    rwlock_unlock_rd(rwlock);
                }

                { // Critical section: write to data, wlock2_wrote, wlock_pass.
                    rwlock_lock_wr(rwlock);
                    *data = (*data > 1000) ? 1 : (*data + 1);
                    *wlock2_wrote = true;
                    *wlock_pass &= (data_value > 0);
                    rwlock_unlock_wr(rwlock);
                }

                TEST_DLOG_VAR_VALUE_HEX("wlock2", "data", data_value);
                std::this_thread::yield();
            } // End while loop
            data_value = 0;

            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

            TEST_DLOG_VAR_VALUE_HEX("end of wlock2", "data", data_value);
        }
    }
    TestTwoThreadReadWaitForOtherWrite::TestTwoThreadReadWaitForOtherWrite(
        Clock &tester_clock) :
        Test("test_two_thread_wait_for_other_write", tester_clock)
    { }
    int TestTwoThreadReadWaitForOtherWrite::run_test_body() {
        using namespace test_two_thread_wait_for_other_write;
        rwlock_t shared_rwlock;
        int data = 1;
        bool wlock_pass = true;
        bool wlock1_wrote = false;
        bool wlock2_wrote = false;
        rwlock_init(&shared_rwlock);
        std::thread thread1(wlock1, &shared_rwlock, &data,
                            &wlock_pass, &wlock1_wrote, &wlock2_wrote);
        std::thread thread2(wlock2, &shared_rwlock, &data,
                            &wlock_pass, &wlock1_wrote, &wlock2_wrote);
        thread1.join();
        thread2.join();
        rwlock_uninit(&shared_rwlock);
        wlock_pass &= (data > 1);
        return (wlock_pass ? 0 : 1);
    }
}
