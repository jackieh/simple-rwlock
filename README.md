# simple-rwlock
Simple subset of pthread read-write lock operations and correctness/performance
testing infrastructure.

The read-write lock implementation is designed to be writer-biased, optimized
for the use case of common read-locks and uncommon write-locks.

### Dependencies

C++17

### Building and running

To build the library, run `make lib`.

To build the executable binary that runs tests, run `make` or `make test`.

The executable binary does not take any command line arguments, so just run `./simple_rwlock_run_tests` to run the tests.
