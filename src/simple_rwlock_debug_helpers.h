#ifndef SIMPLE_RWLOCK_DEBUG_HELPERS_H
#define SIMPLE_RWLOCK_DEBUG_HELPERS_H

#include <assert.h>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>

#include <simple_rwlock.h>

namespace simple_rwlock {
    static std::mutex log_mutex;

#define ASSERT_POSITIVE(cv) assert(cv > 0)
#define ASSERT_LOCKED(m) assert(!m->try_lock())
#define PRINT_CALLED(fn) print_called(fn)
#define PRINT_AWNUM(fn, rwl, co) \
    print_counter(fn, "active writers", co, rwl->num_active_writers)
#define PRINT_ARNUM(fn, rwl, co) \
    print_counter(fn, "active readers", co, rwl->num_active_readers)
#define PRINT_AAWLOCK(fn, mu) print_mutex(fn, mu, "active writers")
#define PRINT_WLOCK(fn, mu) print_mutex(fn, mu, "write")

    // Report a function call.
    inline void print_called(std::string func_name) {
        std::stringstream print_stream;
        print_stream << "(Thread " << std::this_thread::get_id() << ")\t"
            << func_name << ": " << "Called" << std::endl;

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
            << func_name << ": " << "Number of " << counter_name << " "
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

#endif // SIMPLE_RWLOCK_DEBUG_HELPERS_H
