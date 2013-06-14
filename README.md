# Building

Circular buffer library can be built as both a static or dynamic library.

    make libstatic

    make libdynamic

# Testing

Testing requires [CUnit][0] to be linked to the `test` target. By default it looks in `/usr/local/lib` for `libcunit.a`. For most systems, the following should be enough to install CUnit.

1. download CUnit, unpack it, and `cd` into the unpacked directory
1. `./configure && sudo make install`

To run the unit tests:

    make test && ./test

[0]: http://cunit.sourceforge.net/

