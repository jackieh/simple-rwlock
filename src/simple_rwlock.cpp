#ifdef DEBUG
#include <iostream>
#endif

#include <simple_rwlock.h>

namespace simple_rwlock {

    void rwlock_init(rwlock_t *rwlock) {
#ifdef DEBUG
        std::cerr << "rwlock_init: Not yet implemented" << std::endl;
#endif
        (void)rwlock;
        return;
    }

    void rwlock_uninit(rwlock_t *rwlock) {
#ifdef DEBUG
        std::cerr << "rwlock_uninit: Not yet implemented" << std::endl;
#endif
        (void)rwlock;
        return;
    }

    void rwlock_lock_rd(rwlock_t *rwlock) {
#ifdef DEBUG
        std::cerr << "rwlock_lock_rd: Not yet implemented" << std::endl;
#endif
        (void)rwlock;
        return;
    }

    void rwlock_unlock_rd(rwlock_t *rwlock) {
#ifdef DEBUG
        std::cerr << "rwlock_unlock_rd: Not yet implemented" << std::endl;
#endif
        (void)rwlock;
        return;
    }

    void rwlock_lock_wr(rwlock_t *rwlock) {
#ifdef DEBUG
        std::cerr << "rwlock_lock_wr: Not yet implemented" << std::endl;
#endif
        (void)rwlock;
        return;
    }

    void rwlock_unlock_wr(rwlock_t *rwlock) {
#ifdef DEBUG
        std::cerr << "rwlock_unlock_wr: Not yet implemented" << std::endl;
#endif
        (void)rwlock;
        return;
    }
}
