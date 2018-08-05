#ifndef SIMPLE_RWLOCK_H
#define SIMPLE_RWLOCK_H

namespace simple_rwlock {
    typedef struct rwlock_t {
    } rwlock_t;

    void rwlock_init(rwlock_t *);
    void rwlock_uninit(rwlock_t *);
    void rwlock_lock_rd(rwlock_t *);
    void rwlock_unlock_rd(rwlock_t *);
    void rwlock_lock_wr(rwlock_t *);
    void rwlock_unlock_wr(rwlock_t *);
}

#endif // SIMPLE_RWLOCK_H
