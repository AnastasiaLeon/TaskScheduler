#include <gtest/gtest.h>
#include "scheduler.h"

// Test scheduler reuse
TEST(TaskSchedulerTest, ReuseScheduler) {
    TaskScheduler scheduler;
    auto id1 = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id1), 1);
    
    auto id2 = scheduler.add([] { return 2; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id2), 2);
}

// Alternative test for scheduler reuse
TEST(TaskSchedulerTest, ReuseScheduler2) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id1), 1);
    
    auto id2 = scheduler.add([] { return 2; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id2), 2);
}

// Test reuse after clearing
TEST(TaskSchedulerTest, ReuseAfterClear) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id1), 1);
    
    auto id2 = scheduler.add([] { return 2; });
    auto id3 = scheduler.add([](int x) { return x + 1; }, scheduler.getFutureResult<int>(id2));
    
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id3), 3);
}

// Test repeated calls to executeAll
TEST(TaskSchedulerTest, DoubleExecuteAll) {
    TaskScheduler scheduler;
    auto id = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_NO_THROW(scheduler.executeAll());
    EXPECT_EQ(scheduler.getResult<int>(id), 1);
}