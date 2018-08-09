#ifndef SIMPLE_RWLOCK_DEBUG_HELPERS_H
#define SIMPLE_RWLOCK_DEBUG_HELPERS_H

#ifdef DEBUG
#include <assert.h>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>

#include <simple_rwlock.h>

namespace simple_rwlock {
    static std::mutex log_mutex;

#define ASSERT_ZERO(cv) assert(cv == 0)
#define ASSERT_POSITIVE(cv) assert(cv > 0)
#define ASSERT_LOCKED(m) assert(!m->try_lock())
#define ASSERT_UNLOCKED(m) assert_unlocked(m)
#define PRINT_CALLED(fn) print_called(fn)
#define PRINT_AWNUM(fn, rwl, co) \
    print_counter(fn, "active writers", co, rwl->num_active_writers)
#define PRINT_ARNUM(fn, rwl, co) \
    print_counter(fn, "active readers", co, rwl->num_active_readers)
#define PRINT_AAWLOCK(fn, mu) print_mutex(fn, mu, "any active writers")
#define PRINT_WOARLOCK(fn, mu) print_mutex(fn, mu, "write or any read")

    // Assert a mutex is unlocked by trying the lock.
    // After the assertion passes, release the lock.
    inline void assert_unlocked(std::mutex *m) {
        assert(m->try_lock());
        m->unlock();
    }

    // Report a function call.
    inline void print_called(std::string func_name) {
        std::stringstream print_stream;
        print_stream << "(Thread " << std::this_thread::get_id() << ")\t"
            << func_name << ": " << "called" << std::endl;

        { // Critical section: write to stdout.
            log_mutex.lock();
            std::cout << print_stream.str();
            std::cout.flush();
            log_mutex.unlock();
        }
    }

    // Report a counter update.
    inline void print_counter(std::string func_name,
                              std::string counter_name,
                              std::string counter_operation,
                              unsigned long counter_value)
    {
        std::stringstream print_stream;
        print_stream << "(Thread " << std::this_thread::get_id() << ")\t"
            << func_name << ": " << "number of " << counter_name << " "
            << counter_operation << " to " << counter_value << std::endl;

        { // Critical section: write to stdout.
            log_mutex.lock();
            std::cout << print_stream.str();
            std::cout.flush();
            log_mutex.unlock();
        }
    }

    // Report a mutex update.
    inline void print_mutex(std::string func_name,
                            std::string mutex_update,
                            std::string mutex_name)
    {
        std::stringstream print_stream;
        print_stream << "(Thread " << std::this_thread::get_id() << ")\t"
            << func_name << ": " << mutex_update << " "
            << mutex_name << " mutex" << std::endl;

        { // Critical section: write to stdout.
            log_mutex.lock();
            std::cout << print_stream.str();
            std::cout.flush();
            log_mutex.unlock();
        }
    }
}

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

#endif // SIMPLE_RWLOCK_DEBUG_HELPERS_H
