#include <chrono>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <simple_rwlock.h>
#include <simple_rwlock_debug_helpers.h>
#include <simple_rwlock_test/test.h>
#include <simple_rwlock_test/tests/test_common.h>
#include <simple_rwlock_test/tests/multi_thread_tests.h>

namespace simple_rwlock_test {
    using namespace simple_rwlock;
    using namespace test_common;

    // Have 8 reader threads and one writer thread. Don't let the writer
    // start writing until after one of the readers has started reading.
    // Don't let any of the readers let go of the reader lock until
    // all of the readers have reported reading at least once.
    namespace test_many_readers_one_writer {
        // Write to the shared data. Don't start writing until at
        // least one of the reader threads have started reading.
        void write_thread(rwlock_t *data_rwlock,         // Shared
                          rwlock_t *pass_rwlock,         // Shared
                          rwlock_t *num_reads_rwlock,    // Shared
                          rwlock_t *writer_wrote_rwlock, // Shared
                          unsigned int *data,            // Shared
                          bool *pass,                    // Shared
                          size_t *num_reads,             // Shared
                          bool *writer_wrote)            // Shared
        {
            unsigned int data_value = 0;
            size_t num_reads_value = 0;
            TEST_DLOG_THREAD_LAUNCH("write thread");

            // Don't try to acquire write access until after
            // at least one reader has become active.
            while (num_reads_value < 1) {
                { // Critical section: read from number of reads.
                    rwlock_lock_rd(num_reads_rwlock);
                    num_reads_value = *num_reads;
                    TEST_DASSERT(num_reads_value <= 8);
                    rwlock_unlock_rd(num_reads_rwlock);
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(10));
                }
            }

            TEST_DLOG_STR("write thread",
                          "try to acquire write lock for data");
            { // Critical section: read from and write to data.
                // We forced all reader threads to be slow so we
                // expect the write lock to not be acquired until
                // after all readers have completed.
                rwlock_lock_wr(data_rwlock);
                data_value = *data;
                *data = 0xfeedcafe;

                { // Critical section: write to writer_wrote_value.
                    rwlock_lock_wr(writer_wrote_rwlock);
                    TEST_DASSERT(!(*writer_wrote));
                    *writer_wrote = true;
                    rwlock_unlock_wr(writer_wrote_rwlock);
                }

                // Slow down the writer after prompting the readers to
                // try reading again, before releasing write access.
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                rwlock_unlock_wr(data_rwlock);
            }

            { // Critical section: read from and write to pass.
                // Make sure the value before the write was the original value.
                rwlock_lock_wr(pass_rwlock);
                *pass &= (data_value == 0xdeadbeef);
                TEST_DASSERT(data_value == 0xdeadbeef);
                rwlock_unlock_wr(pass_rwlock);
            }
        }

        // Read from shared data. Don't release read lock until
        // all other readers have reported reading at least once.
        void read_thread(unsigned int thread_num,       // Not shared
                         rwlock_t *data_rwlock,         // Shared
                         rwlock_t *pass_rwlock,         // Shared
                         rwlock_t *num_reads_rwlock,    // Shared
                         rwlock_t *writer_wrote_rwlock, // Shared
                         unsigned int *data,            // Shared
                         bool *pass,                    // Shared
                         size_t *num_reads,             // Shared
                         bool *writer_wrote)            // Shared
        {
            std::stringstream thread_name_stream;
            thread_name_stream << "read thread #" << thread_num;
            std::string thread_name = thread_name_stream.str();
            TEST_DLOG_THREAD_LAUNCH(thread_name);

            unsigned int data_value = 0;
            size_t num_reads_value = 0;
            bool writer_wrote_value = false;
            bool first_reader = false;

            TEST_DLOG_STR(thread_name, "try to acquire read lock for data");
            { // Critical section: read from data.
                rwlock_lock_rd(data_rwlock);

                // Make the read process very slow.
                std::this_thread::sleep_for(
                    std::chrono::microseconds(10 * thread_num));

                // Report the read has been performed.
                TEST_DLOG_STR(thread_name,
                              "try to acquire write lock for num_reads");
                { // Critical section: write to and read from num_reads.
                    rwlock_lock_wr(num_reads_rwlock);
                    TEST_DASSERT(*num_reads < 8);
                    first_reader = (*num_reads == 0);
                    *num_reads = *num_reads + 1;
                    num_reads_value = *num_reads;
                    rwlock_unlock_wr(num_reads_rwlock);
                }
                TEST_DLOG_VAR_VALUE_DEC(
                    thread_name, "new value of num_reads", num_reads_value);

                // Pause between reporting the read and performing the read.
                std::this_thread::sleep_for(
                    std::chrono::microseconds(10 * thread_num));

                // If this isn't the first reader, then the writer may have
                // started waiting to write before this reader received read
                // access (because the rwlock is writer-biased). Otherwise we
                // assume the writer hasn't written yet.
                if (first_reader) {
                    // For a few iterations, repeatedly confirm data
                    // has not changed from its original value.
                    for (int i = 0; i < 10; i++) {
                        { // Critical section: write to and read from pass.
                            rwlock_lock_wr(pass_rwlock);
                            TEST_DASSERT(*data == 0xdeadbeef);
                            *pass &= (*data == 0xdeadbeef);
                            rwlock_unlock_wr(pass_rwlock);
                            std::this_thread::sleep_for(
                                std::chrono::microseconds(10));
                        }
                        TEST_DLOG_VAR_VALUE_HEX(thread_name, "data", *data);
                        std::this_thread::yield();
                    } // End while loop
                }

                TEST_DLOG_STR(thread_name, "release read lock to data");
                rwlock_unlock_rd(data_rwlock);
            } // End critical section with read access to data.

            while (!writer_wrote_value) {
                { // Critical section: read from writer_wrote_value.
                    rwlock_lock_wr(writer_wrote_rwlock);
                    writer_wrote_value = *writer_wrote;
                    rwlock_unlock_wr(writer_wrote_rwlock);
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(10));
                }
            }

            // Check for the new written value in data.
            { // Critical section: read from data.
                rwlock_lock_rd(data_rwlock);
                data_value = *data;
                rwlock_unlock_rd(data_rwlock);
            }

            { // Critical section: read from and write to pass.
                rwlock_lock_wr(pass_rwlock);
                TEST_DASSERT(data_value == 0xfeedcafe);
                *pass &= (data_value == 0xfeedcafe);
                rwlock_unlock_wr(pass_rwlock);
            }

            TEST_DLOG_STR(thread_name, "wait for other readers to complete");
            while (num_reads_value < 8) {
                { // Critical section: read from num_reads.
                    rwlock_lock_rd(num_reads_rwlock);
                    num_reads_value = *num_reads;
                    rwlock_unlock_rd(num_reads_rwlock);
                }
                std::this_thread::yield();
            } // End while loop

            TEST_DLOG_VAR_VALUE_HEX(
                "end of " + thread_name, "data", data_value);
        }
    }
    TestManyReadersOneWriter::TestManyReadersOneWriter(Clock &tester_clock) :
        Test("test_many_readers_one_writer", tester_clock)
    { }
    int TestManyReadersOneWriter::run_test_body() {
        using namespace test_many_readers_one_writer;
        rwlock_t data_rwlock;
        rwlock_t pass_rwlock;
        rwlock_t num_reads_rwlock;
        rwlock_t writer_wrote_rwlock;
        unsigned int data = 0xdeadbeef;
        bool pass = true;
        size_t num_reads = 0;
        bool writer_wrote = false;
        rwlock_init(&data_rwlock);
        rwlock_init(&pass_rwlock);
        rwlock_init(&num_reads_rwlock);
        rwlock_init(&writer_wrote_rwlock);
        std::thread writer(write_thread, &data_rwlock, &pass_rwlock,
                           &num_reads_rwlock, &writer_wrote_rwlock,
                           &data, &pass, &num_reads, &writer_wrote);
        std::thread reader1(read_thread, 1, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader2(read_thread, 2, &data_rwlock,&pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader3(read_thread, 3, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader4(read_thread, 4, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader5(read_thread, 5, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader6(read_thread, 6, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader7(read_thread, 7, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        std::thread reader8(read_thread, 8, &data_rwlock, &pass_rwlock,
                            &num_reads_rwlock, &writer_wrote_rwlock,
                            &data, &pass, &num_reads, &writer_wrote);
        writer.join();
        reader1.join();
        reader2.join();
        reader3.join();
        reader4.join();
        reader5.join();
        reader6.join();
        reader7.join();
        reader8.join();
        rwlock_uninit(&data_rwlock);
        rwlock_uninit(&pass_rwlock);
        rwlock_uninit(&num_reads_rwlock);
        rwlock_uninit(&writer_wrote_rwlock);
        pass &= (data == 0xfeedcafe);
        return (pass ? 0 : 1);
    }
}
