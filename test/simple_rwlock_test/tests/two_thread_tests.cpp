#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include <simple_rwlock.h>
#include <simple_rwlock_debug_helpers.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/two_thread_tests.h>

using namespace simple_rwlock;

namespace simple_rwlock_test {
    // Functions run by threads in the TestTwoThreadReadOnceEach test class.
    namespace test_two_thread_read_once_each {
        // Read once from shared data and confirm it is the correct value.
        void rlock1(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock1_pass)  // Not shared
        {
            unsigned int data_value = 0;

#ifdef DEBUG
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\tLaunched running rlock1" << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif

            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

#ifdef DEBUG
            print_stream = std::stringstream();
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\trlock1: data is 0x"
                << std::hex << data_value << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif

            *rlock1_pass &= (data_value == 0xdeadbeef);
        }

        // Read once from shared data and confirm it is the correct value.
        void rlock2(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock2_pass)  // Not shared
        {
            unsigned int data_value = 0;

#ifdef DEBUG
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\tLaunched running rlock2" << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif

            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

#ifdef DEBUG
            print_stream = std::stringstream();
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\trlock2: data is 0x"
                << std::hex << data_value << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif

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
            unsigned int data_value = 0;
            bool rlock2_read_value = false;

#ifdef DEBUG
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\tLaunched running rlock1" << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif

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

#ifdef DEBUG
                print_stream = std::stringstream();
                print_stream << "(Thread " << std::this_thread::get_id()
                    << ")\trlock1: data is 0x"
                    << std::hex << data_value << std::endl;
                { // Critical section: write to stdout.
                    log_mutex.lock();
                    std::cout << print_stream.str();
                    std::cout.flush();
                    log_mutex.unlock();
                }
#endif

                std::this_thread::yield();
            } // End while loop
            data_value = 0;

            { // Critical section: read from data
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

#ifdef DEBUG
            print_stream = std::stringstream();
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\trlock1: data is still 0x"
                << std::hex << data_value << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif
        }

        // Repeatedly read from shared data until rlock1
        // reports having read the data at least once.
        void rlock2(rwlock_t *rwlock,   // Shared
                    unsigned int *data, // Shared
                    bool *rlock_pass,   // Shared
                    bool *rlock1_read,  // Shared
                    bool *rlock2_read)  // Shared
        {
            unsigned int data_value = 0;
            bool rlock1_read_value = false;

#ifdef DEBUG
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\tLaunched running rlock2" << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif

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

#ifdef DEBUG
                print_stream = std::stringstream();
                print_stream << "(Thread " << std::this_thread::get_id()
                    << ")\trlock2: data is 0x"
                    << std::hex << data_value << std::endl;
                { // Critical section: write to stdout.
                    log_mutex.lock();
                    std::cout << print_stream.str();
                    std::cout.flush();
                    log_mutex.unlock();
                }
#endif

                std::this_thread::yield();
            } // End while loop
            data_value = 0;

            { // Critical section: read from data.
                rwlock_lock_rd(rwlock);
                data_value = *data;
                rwlock_unlock_rd(rwlock);
            }

#ifdef DEBUG
            print_stream = std::stringstream();
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\trlock2: data is still 0x"
                << std::hex << data_value << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
#endif
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
        return 0;
    }
}
