#include <simple_rwlock.h>
#include <simple_rwlock_test/tester.h>

int main() {
    simple_rwlock_test::Tester tester;
    return tester.run_tests();
}
