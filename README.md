# Building

To build simply:

    make

# Testing

Testing requires CUnit to be linked to the `test` target. By default it looks in `/usr/local/lib` for `libcunit.a`.

To run the tests:

    make test && ./test
