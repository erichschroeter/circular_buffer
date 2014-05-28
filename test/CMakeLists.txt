set(TEST_SRCS
test.cpp
)

# include the configured test.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(run_tests ${TEST_SRCS})
target_link_libraries(run_tests cb)
