#include <mutex>

#include <simple_rwlock.h>

#ifdef DEBUG
#include <simple_rwlock_debug_helpers.h>
#else // DEBUG
// Effectively erase any of these macro calls if not debugging.
#define ASSERT_POSITIVE(cv)
#define ASSERT_LOCKED(m)
#define PRINT_CALLED(fn)
#define PRINT_AWNUM(fn, rwl, co)
#define PRINT_ARNUM(fn, rwl, co)
#define PRINT_AAWLOCK(fn, mu)
#define PRINT_WLOCK(fn, mu)
#endif // DEBUG

namespace simple_rwlock {
    void rwlock_init(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_init");
        rwlock->num_active_readers = 0;
        rwlock->num_active_writers = 0;
        rwlock->write_mutex = new std::mutex;
        rwlock->num_active_writers_mutex = new std::mutex;
        rwlock->any_active_writers_mutex = new std::mutex;
        rwlock->num_active_readers_mutex = new std::mutex;
    }

    void rwlock_uninit(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_uninit");
        delete rwlock->write_mutex;
        delete rwlock->num_active_writers_mutex;
        delete rwlock->any_active_writers_mutex;
        delete rwlock->num_active_readers_mutex;
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
        PRINT_CALLED("rwlock_lock_rd");
        PRINT_AAWLOCK("rwlock_lock_rd", "Acquiring");
        rwlock->any_active_writers_mutex->lock();
        PRINT_AAWLOCK("rwlock_lock_rd", "Locked");
        rwlock->num_active_readers_mutex->lock();
        rwlock->num_active_readers++;
        PRINT_ARNUM("rwlock_lock_rd", rwlock, "incremented");
        rwlock->num_active_readers_mutex->unlock();
        PRINT_AAWLOCK("rwlock_lock_rd", "Releasing");
        rwlock->any_active_writers_mutex->unlock();
    }

    // Atomically decrement the number of active readers.
    void rwlock_unlock_rd(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_unlock_rd");
        rwlock->num_active_readers_mutex->lock();
        ASSERT_POSITIVE(rwlock->num_active_readers);
        rwlock->num_active_readers--;
        PRINT_ARNUM("rwlock_unlock_rd", rwlock, "decremented");
        rwlock->num_active_readers_mutex->unlock();
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
        PRINT_CALLED("rwlock_lock_wr");
        rwlock->num_active_writers_mutex->lock();
        if (rwlock->num_active_writers == 0) {
            PRINT_AAWLOCK("rwlock_lock_wr", "Acquiring");
            rwlock->any_active_writers_mutex->lock();
            PRINT_AAWLOCK("rwlock_lock_wr", "Locked");
        }
        ASSERT_LOCKED(rwlock->any_active_writers_mutex);
        rwlock->num_active_writers++;
        PRINT_AWNUM("rwlock_lock_wr", rwlock, "incremented");
        rwlock->num_active_writers_mutex->unlock();
        PRINT_WLOCK("rwlock_lock_wr", "Acquiring");
        rwlock->write_mutex->lock();
        PRINT_WLOCK("rwlock_lock_wr", "Locked");
    }

    // Atomically decrement the number of writers counter. Before
    // releasing access to the number of writers counter, release
    // the any_active_writers lock if and only if the new value of
    // the counter is zero.
    void rwlock_unlock_wr(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_unlock_wr");
        rwlock->write_mutex->unlock();
        rwlock->num_active_writers_mutex->lock();
        ASSERT_POSITIVE(rwlock->num_active_writers);
        rwlock->num_active_writers--;
        PRINT_AWNUM("rwlock_unlock_wr", rwlock, "decremented");
        if (rwlock->num_active_writers == 0) {
            PRINT_AAWLOCK("rwlock_unlock_wr", "Releasing");
            rwlock->any_active_writers_mutex->unlock();
        }
        PRINT_AAWLOCK("rwlock_unlock_wr", "Releasing");
        rwlock->num_active_writers_mutex->unlock();
    }
}
