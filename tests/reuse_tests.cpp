#include <gtest/gtest.h>
#include "scheduler.h"

//Проверка переиспользования планировщика
TEST(TaskSchedulerTest, ReuseScheduler) {
    TaskScheduler scheduler;
    auto id1 = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id1), 1);
    
    auto id2 = scheduler.add([] { return 2; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id2), 2);
}

//Альтернативная проверка переиспользования
TEST(TaskSchedulerTest, ReuseScheduler2) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id1), 1);
    
    auto id2 = scheduler.add([] { return 2; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id2), 2);
}

//Проверка повторного выполнения после очистки
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

//Проверка повторного вызова executeAll
TEST(TaskSchedulerTest, DoubleExecuteAll) {
    TaskScheduler scheduler;
    auto id = scheduler.add([] { return 1; });
    scheduler.executeAll();
    EXPECT_NO_THROW(scheduler.executeAll());
    EXPECT_EQ(scheduler.getResult<int>(id), 1);
}