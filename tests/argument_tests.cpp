#include <gtest/gtest.h>
#include "scheduler.h"

// Test maximum number of arguments (2)
TEST(TaskSchedulerTest, TwoArguments) {
    TaskScheduler scheduler;
    auto id = scheduler.add([](int a, float b) { return a + b; }, 1, 2.5f);
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 3.5f);
}

// Test task with one argument
TEST(TaskSchedulerTest, OneArgument) {
    TaskScheduler scheduler;
    auto id = scheduler.add([](int a) { return a * 2; }, 5);
    EXPECT_EQ(scheduler.getResult<int>(id), 10);
}

// Test task with no arguments
TEST(TaskSchedulerTest, ZeroArguments) {
    TaskScheduler scheduler;
    auto id = scheduler.add([] { return 42; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 42);
}

// Test handling of constant arguments
TEST(TaskSchedulerTest, ConstArguments) {
    TaskScheduler scheduler;
    
    const int x = 5;
    auto id = scheduler.add([](const int& v) { return v * 2; }, x);
    
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 10);
}

// Test passing argument by reference
TEST(TaskSchedulerTest, ReferenceArgument) {
    TaskScheduler scheduler;
    int value = 10;
    auto id = scheduler.add([](int& v) { return ++v; }, std::ref(value));
    scheduler.executeAll();
    EXPECT_EQ(value, 11);
}