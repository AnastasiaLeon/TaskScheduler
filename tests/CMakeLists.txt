cmake_minimum_required(VERSION 3.12)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/type_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/argument_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cycle_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/reuse_tests.cpp
)

add_executable(task_scheduler_test ${TEST_SOURCES})

target_include_directories(
    task_scheduler_test
    PRIVATE ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(
    task_scheduler_test
    PRIVATE
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(task_scheduler_test)