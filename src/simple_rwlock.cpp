#ifdef DEBUG
#include <assert.h>
#include <iostream>
#include <string>
#endif

#include <mutex>

#include <simple_rwlock.h>

namespace simple_rwlock {
#ifdef DEBUG
#define ASSERT_POSITIVE(cv) assert(cv > 0)
#define ASSERT_LOCKED(m) assert(!m.try_lock())

    static std::mutex log_mutex;

    inline void print_func(std::string func_name, rwlock_t *rwlock) {
        std::cout << func_name << " (" << (void *)rwlock << "): ";
    }

    // Report a function call.
    inline void print_called(std::string func_name, rwlock_t *rwlock) {
        std::lock_guard<std::mutex> lock(log_mutex);
        print_func(func_name, rwlock);
        std::cout << "Called" << std::endl;
        std::cout.flush();
    }

#define PRINT_CALLED(fn, rwl) print_called(fn, rwl)

    // Report a counter update.
    inline void print_counter(std::string func_name, rwlock_t *rwlock,
                              std::string counter_name,
                              std::string counter_operation,
                              unsigned long counter_value)
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        print_func(func_name, rwlock);
        std::cout << "Number of " << counter_name << " "
            << counter_operation << " to " << counter_value << std::endl;
        std::cout.flush();
    }

#define PRINT_AWNUM(fn, rwl, co) \
    print_counter(fn, rwl, "active writers", co, rwl->num_active_writers)
#define PRINT_ARNUM(fn, rwl, co) \
    print_counter(fn, rwl, "active readers", co, rwl->num_active_readers)

    // Report a mutex update.
    inline void print_mutex(std::string func_name, rwlock_t *rwlock,
                            std::string mutex_update,
                            std::string mutex_name)
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        print_func(func_name, rwlock);
        std::cout << mutex_update << " " << mutex_name
            << " mutex" << std::endl;
        std::cout.flush();
    }

#define PRINT_AAWLOCK(fn, rwl, mu) print_mutex(fn, rwl, mu, "active writers")
#define PRINT_WLOCK(fn, rwl, mu) print_mutex(fn, rwl, mu, "write")

#else // DEBUG
// Effectively erase any of these macro calls if not testing/debugging.
#define ASSERT_POSITIVE(cv)
#define ASSERT_LOCKED(m)
#define PRINT_CALLED(fn, rwl)
#define PRINT_AWNUM(fn, rwl, co)
#define PRINT_ARNUM(fn, rwl, co)
#define PRINT_AAWLOCK(fn, rwl, mu)
#define PRINT_WLOCK(fn, rwl, mu)
#endif // DEBUG

    void rwlock_init(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_init", rwlock);
        rwlock->num_active_readers = 0;
        rwlock->num_active_writers = 0;
    }

    void rwlock_uninit(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_uninit", rwlock);
        rwlock->write_mutex.unlock();
        rwlock->num_active_readers_mutex.unlock();
        rwlock->num_active_writers_mutex.unlock();
    }

    // In order to read, the following must be the case:
    // - No writers are active. In other words no writers
    //   are either writing or waiting to write.
    // - The number of readers is atomically incremented
    //   before reading begins.
    //
    // Do the following:
    // - Wait for no writers to be active by acquiring the
    //   any_active_writers lock.
    // - Atomically increment number of readers count.
    // - Release any_active_writers lock to allow any
    //   writers to start waiting to write.
    void rwlock_lock_rd(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_lock_rd", rwlock);
        PRINT_AAWLOCK("rwlock_lock_rd", rwlock, "Acquiring");
        rwlock->any_active_writers_mutex.lock();
        rwlock->num_active_readers_mutex.lock();
        rwlock->num_active_readers++;
        PRINT_ARNUM("rwlock_lock_rd", rwlock, "incremented");
        rwlock->num_active_readers_mutex.unlock();
        rwlock->any_active_writers_mutex.unlock();
    }

    // Atomically decrement the number of active readers.
    void rwlock_unlock_rd(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_unlock_rd", rwlock);
        rwlock->num_active_readers_mutex.lock();
        ASSERT_POSITIVE(rwlock->num_active_readers);
        rwlock->num_active_readers--;
        PRINT_ARNUM("rwlock_unlock_rd", rwlock, "decremented");
        rwlock->num_active_readers_mutex.unlock();
    }

    // In order to wait to write, the following must be the case:
    // - No readers are active.
    // - The any_active_writers mutex is locked to keep out any
    //   new readers.
    // - The number of active writers is atomically incremented
    //   before starting to wait for write access.
    //
    // In order to write, the following must be the case:
    // - No readers are active.
    // - The any_active_writers mutex is locked to keep out any
    //   new readers.
    // - No other writers are writing.
    //
    // Do the following:
    // - Acquire exclusive access to the number of active writers counter.
    // - Either
    //   * Case 1: No writers were active before the exclusive access to
    //             the active writers counter, so signal that active writers
    //             exist by acquiring the any_active_writers lock, and
    //             atomically increment the number of writers counter.
    //   * Case 2: Other active writers exist, so some other process has
    //             already acquired the any_active_writers lock. Just
    //             atomically increment the number of writers counter.
    // - Release the exclusive access to the number of active writers
    //   counter so other writers can start waiting, then wait for the
    //   actual write lock.
    void rwlock_lock_wr(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_lock_wr", rwlock);
        rwlock->num_active_writers_mutex.lock();
        if (rwlock->num_active_writers == 0) {
            PRINT_AAWLOCK("rwlock_lock_wr", rwlock, "Acquiring");
            rwlock->any_active_writers_mutex.lock();
            PRINT_AAWLOCK("rwlock_lock_wr", rwlock, "Locked");
        }
        ASSERT_LOCKED(rwlock->any_active_writers_mutex);
        rwlock->num_active_writers++;
        PRINT_AWNUM("rwlock_lock_wr", rwlock, "incremented");
        rwlock->num_active_writers_mutex.unlock();
        PRINT_WLOCK("rwlock_lock_wr", rwlock, "Acquiring");
        rwlock->write_mutex.lock();
        PRINT_WLOCK("rwlock_lock_wr", rwlock, "Locked");
    }

    // Atomically decrement the number of writers counter. Before
    // releasing access to the number of writers counter, release
    // the any_active_writers lock if and only if the new value of
    // the counter is zero.
    void rwlock_unlock_wr(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_unlock_wr", rwlock);
        rwlock->write_mutex.unlock();
        rwlock->num_active_writers_mutex.lock();
        ASSERT_POSITIVE(rwlock->num_active_writers);
        rwlock->num_active_writers--;
        PRINT_AWNUM("rwlock_unlock_wr", rwlock, "decremented");
        if (rwlock->num_active_writers == 0) {
            PRINT_AAWLOCK("rwlock_unlock_wr", rwlock, "Releasing");
            rwlock->any_active_writers_mutex.unlock();
        }
        PRINT_AAWLOCK("rwlock_unlock_wr", rwlock, "Releasing");
        rwlock->num_active_writers_mutex.unlock();
    }
}
