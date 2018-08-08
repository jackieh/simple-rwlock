#ifndef SIMPLE_RWLOCK_H
#define SIMPLE_RWLOCK_H

#include <mutex>

namespace simple_rwlock {
#ifdef DEBUG
    // Make any underflow bugs visible if debugging/testing.
    typedef signed long rwlock_count_t;
#else
    typedef unsigned long rwlock_count_t;
#endif

    typedef struct rwlock_t {
        // At any given time, at most one writer may have write access.
        std::mutex *write_mutex;
        // At any given time, more than one writer may be active.
        // A writer is active when it is either writing or waiting to write.
        rwlock_count_t num_active_writers;
        std::mutex *num_active_writers_mutex;
        std::mutex *any_active_writers_mutex;
        // At any given time, more than one reader may be active.
        // A reader is active when it has permission to read.
        rwlock_count_t num_active_readers;
        std::mutex *num_active_readers_mutex;
    } rwlock_t;

    void rwlock_init(rwlock_t *);
    void rwlock_uninit(rwlock_t *);
    void rwlock_lock_rd(rwlock_t *);
    void rwlock_unlock_rd(rwlock_t *);
    void rwlock_lock_wr(rwlock_t *);
    void rwlock_unlock_wr(rwlock_t *);
}

#endif // SIMPLE_RWLOCK_H
