#ifndef SRWLT_TEST_COMMON_H
#define SRWLT_TEST_COMMON_H

#ifdef DEBUG
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <simple_rwlock_debug_helpers.h>
#endif // DEBUG
#include <simple_rwlock.h>

namespace simple_rwlock_test {
    using namespace simple_rwlock;
    namespace test_common {

#ifdef DEBUG
        inline void print_thread_launch(std::string thread_name) {
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\t" << thread_name << ": launched" << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
        }

        inline void print_var_value(
            std::string thread_name, std::string var_name,
            unsigned int var_value,
            std::string value_prefix,
            std::ios_base& print_fmt (std::ios_base& str))
        {
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\t" << thread_name << ": " << var_name << " is "
                << value_prefix << print_fmt << var_value << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
        }

        inline void print_string(std::string thread_name,
                                 std::string expr)
        {
            std::stringstream print_stream;
            print_stream << "(Thread " << std::this_thread::get_id()
                << ")\t" << thread_name << ": " << expr << std::endl;
            { // Critical section: write to stdout.
                log_mutex.lock();
                std::cout << print_stream.str();
                std::cout.flush();
                log_mutex.unlock();
            }
        }

// Debug log macros.
#define TEST_DLOG_THREAD_LAUNCH(tn) print_thread_launch(tn)
#define TEST_DLOG_VAR_VALUE_HEX(tn, vn, vv) \
        print_var_value(tn, vn, vv, "0x", std::hex)
#define TEST_DLOG_VAR_VALUE_DEC(tn, vn, vv) \
        print_var_value(tn, vn, vv, "", std::dec)
#define TEST_DLOG_STR(tn, s) print_string(tn, s)

// Debug assert macros.
#define TEST_DASSERT(expr) assert(expr)

#else // DEBUG

// Effectively erase any of these macro calls if not debugging.
#define TEST_DLOG_THREAD_LAUNCH(tn)
#define TEST_DLOG_VAR_VALUE_HEX(tn, vn, vv)
#define TEST_DLOG_VAR_VALUE_DEC(tn, vn, vv)
#define TEST_DLOG_STR(tn, s)
#define TEST_DASSERT(expr)

#endif // DEBUG

    } // End of test_common namespace
} // End of simple_rwlock_test namespace

#endif // SRWLT_TEST_COMMON_H
