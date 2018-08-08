#include <mutex>

#include <simple_rwlock.h>

#ifdef DEBUG
#include <simple_rwlock_debug_helpers.h>
#else // DEBUG
// Effectively erase any of these macro calls if not debugging.
#define ASSERT_ZERO(cv)
#define ASSERT_POSITIVE(cv)
#define ASSERT_LOCKED(m)
#define ASSERT_UNLOCKED(m)
#define PRINT_CALLED(fn)
#define PRINT_AWNUM(fn, rwl, co)
#define PRINT_ARNUM(fn, rwl, co)
#define PRINT_AAWLOCK(fn, mu)
#define PRINT_WOARLOCK(fn, mu)
#endif // DEBUG

namespace simple_rwlock {
    void rwlock_init(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_init");
        rwlock->num_active_readers = 0;
        rwlock->num_active_writers = 0;
        rwlock->write_or_any_read_mutex = new std::mutex;
        rwlock->num_active_writers_mutex = new std::mutex;
        rwlock->any_active_writers_mutex = new std::mutex;
        rwlock->num_active_readers_mutex = new std::mutex;
    }

    void rwlock_uninit(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_uninit");
        delete rwlock->write_or_any_read_mutex;
        delete rwlock->num_active_writers_mutex;
        delete rwlock->any_active_writers_mutex;
        delete rwlock->num_active_readers_mutex;
    }

    //--------------------------------------------------------------------------
    // Requirement: The rwlock object is writer-biased, so any active writers
    //              (either writing or waiting to write) must become inactive
    //              before the reader can establish read access.
    // Enforcement: The any_active_writers mutex must become locked.
    //--------------------------------------------------------------------------
    // Requirement: No writers have write access between the time any reader
    //              has established read access and the time all readers are
    //              no longer reading.
    // Enforcement: The write_or_any_read mutex must be locked. Either this
    //              reader will lock the write_or_any_read mutex or some other
    //              reader has already locked the write_or_any_read mutex when
    //              it became active.
    //--------------------------------------------------------------------------
    // Requirement: This function call must accurately determine whether or not
    //              to lock the write_or_any_read mutex.
    // Enforcement: The number of active readers is checked for zero while the
    //              mutex protecting the number of active readers counter is
    //              locked. If it is zero then lock the write_or_any_read
    //              mutex.
    //--------------------------------------------------------------------------
    // Requirement: Future calls to the rwlock_lock_rd function must have the
    //              information needed to accurately determine whether to lock
    //              the write_or_any_read mutex.
    // Requirement: Future calls to the rwlock_unlock_rd function must have the
    //              information needed to accurately determine whether to
    //              unlock the write_or_any_read mutex.
    // Enforcement: The number of readers counter is incremented while the
    //              mutex protecting the number of readers counter is locked.
    //--------------------------------------------------------------------------
    // Requirement: Other readers may start reading between the time this
    //              reader has established its read access and the time this
    //              reader has released its read access.
    // Enforcement: Release the mutex protecting the number of readers counter
    //              by the time this function completes execution.
    //--------------------------------------------------------------------------
    // Requirement: Writers are able to start waiting to write between the
    //              time a reader has established read access and the time
    //              reading completes.
    // Enforcement: Release the any_active_writers mutex after the
    //              write_or_any_read_mutex is locked and before this
    //              function completes execution.
    //--------------------------------------------------------------------------
    void rwlock_lock_rd(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_lock_rd");
        PRINT_AAWLOCK("rwlock_lock_rd", "Acquiring");
        rwlock->any_active_writers_mutex->lock();
        PRINT_AAWLOCK("rwlock_lock_rd", "Locked");
        rwlock->num_active_readers_mutex->lock();
        if (rwlock->num_active_readers == 0) {
            PRINT_WOARLOCK("rwlock_lock_rd", "Acquiring");
            rwlock->write_or_any_read_mutex->lock();
            PRINT_WOARLOCK("rwlock_lock_rd", "Locked");
        }
        ASSERT_LOCKED(rwlock->write_or_any_read_mutex);
        rwlock->num_active_readers++;
        PRINT_ARNUM("rwlock_lock_rd", rwlock, "incremented");
        rwlock->num_active_readers_mutex->unlock();
        PRINT_AAWLOCK("rwlock_lock_rd", "Releasing");
        rwlock->any_active_writers_mutex->unlock();
    }

    //--------------------------------------------------------------------------
    // Requirement: Waiting writers must be able to establish write access
    //              after the last active reader has released its read access.
    // Requirement: Readers must be able to establish read access after the
    //              last active reader has released read access.
    // Enforcement: The write_or_any_read mutex is unlocked if no more
    //              readers are active.
    //--------------------------------------------------------------------------
    // Requirement: Future calls to the rwlock_lock_rd function must have the
    //              information needed to accurately determine whether to
    //              lock the write_or_any_read mutex.
    // Requirement: Future calls to the rwlock_unlock_rd function must have
    //              the information needed to accurately determine whether to
    //              unlock the write_or_any_read mutex.
    // Enforcement: The number of readers is decremented while the mutex
    //              protecting the number of readers counter is locked.
    //--------------------------------------------------------------------------
    // Requirement: This function call must accurately determine whether or
    //              not to release the write_or_any_read mutex.
    // Enforcement: The number of readers is checked for zero while the mutex
    //              protecting the number of readers counter is locked. If it
    //              is zero then unlock the write_or_any_read mutex.
    //--------------------------------------------------------------------------
    // Requirement: Readers must be able to establish read access after any
    //              active reader has released its read access.
    // Enforcement: Release the mutex protecting the number of active readers
    //              counter by the time this function completes execution.
    //--------------------------------------------------------------------------
    void rwlock_unlock_rd(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_unlock_rd");
        rwlock->num_active_readers_mutex->lock();
        ASSERT_POSITIVE(rwlock->num_active_readers);
        rwlock->num_active_readers--;
        PRINT_ARNUM("rwlock_unlock_rd", rwlock, "decremented");
        if (rwlock->num_active_readers == 0) {
            PRINT_WOARLOCK("rwlock_unlock_rd", "Releasing");
            rwlock->write_or_any_read_mutex->unlock();
        }
        rwlock->num_active_readers_mutex->unlock();
    }

    //--------------------------------------------------------------------------
    // Requirement: The rwlock object is writer-biased, so readers must not be
    //              able to become active between the time this writer has
    //              started waiting and the time it has released write access.
    // Enforcement: The any_active_writers mutex must be locked. Either this
    //              writer will lock the any_active_writers mutex or some other
    //              writer has already any_active_writers mutex when it became
    //              active.
    //--------------------------------------------------------------------------
    // Requirement: This function call must accurately determine whether or not
    //              to lock the any_active_writers mutex.
    // Enforcement: The number of active writers is checked for zero after
    //              locking the mutex protecting the number of active writers
    //              counter. If it is zero then lock the any_active_writers
    //              mutex.
    //--------------------------------------------------------------------------
    // Requirement: Future calls to the rwlock_lock_wr function must have the
    //              information needed to accurately determine whether to lock
    //              the any_active_writers mutex.
    // Requirement: Future calls to the rwlock_unlock_wr function must have the
    //              information needed to accurately determine whether to
    //              unlock the any_active_writers mutex.
    // Enforcement: The number of active writers counter is incremented while
    //              the mutex protecting the number of active writers counter
    //              is locked.
    //--------------------------------------------------------------------------
    // Requirement: The writer may not have write access while any other
    //              writers have write access.
    // Requirement: The write may not have write access while any readers
    //              are active.
    // Requirement: The writer must establish write access by the time this
    //              function completes execution.
    // Enforcement: Always lock the write_or_any_read mutex in this funciton.
    //--------------------------------------------------------------------------
    // Requirement: Other writers may start waiting to write between the time
    //              this writer starts waiting and the time this writer
    //              establishes write access.
    // Enforcement: Release the mutex protecting the number of writers counter
    //              before locking the write access mutex.
    //--------------------------------------------------------------------------
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
        PRINT_WOARLOCK("rwlock_lock_wr", "Acquiring");
        rwlock->write_or_any_read_mutex->lock();
        PRINT_WOARLOCK("rwlock_lock_wr", "Locked");
        ASSERT_ZERO(rwlock->num_active_readers);
    }

    //--------------------------------------------------------------------------
    // Requirement: Waiting writers must be able to establish write access
    //              after a writer has released write access.
    // Requirement: Readers must potentially be able to establish read
    //              access after a writer has released write access.
    // Enforcement: Always unlock the write_or_any_read mutex.
    //--------------------------------------------------------------------------
    // Requirement: Readers must be able to establish read access after the
    //              last active writer has released write access.
    // Enforcement: The any_active_writers mutex is unlocked if no more
    //              writers are active.
    //--------------------------------------------------------------------------
    // Requirement: Future calls to the rwlock_lock_wr function must have the
    //              information needed to accurately determine whether to lock
    //              the any_active_writers mutex.
    // Requirement: Future calls to the rwlock_unlock_wr function must have the
    //              information needed to accurately determine whether to
    //              unlock the any_active_writers mutex.
    // Enforcement: The number of active writers counter is decremented while
    //              the mutex protecting the number of active writers counter
    //              is locked.
    //--------------------------------------------------------------------------
    // Requirement: This function call must accurately determine whether or not
    //              to unlock the any_active_writers mutex.
    // Enforcement: Check the number of active writers while the mutex
    //              protecting the number of active writers counter is locked.
    //              If it is zero then unlock the any_active_writers mutex.
    //--------------------------------------------------------------------------
    // Requirement: Writers must be able to start waiting to write after any
    //              writer with write access has released its write access.
    // Enforcement: Release the mutex protecting the number of active writers
    //              counter by the time this function completes execution.
    //--------------------------------------------------------------------------
    void rwlock_unlock_wr(rwlock_t *rwlock) {
        PRINT_CALLED("rwlock_unlock_wr");
        ASSERT_ZERO(rwlock->num_active_readers);
        ASSERT_LOCKED(rwlock->write_or_any_read_mutex);
        rwlock->write_or_any_read_mutex->unlock();
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
