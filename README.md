# simple-rwlock
Simple subset of pthread read-write lock operations and correctness/performance
testing infrastructure.

The read-write lock implementation is designed to be writer-biased, optimized
for the use case of common read-locks and uncommon write-locks.
