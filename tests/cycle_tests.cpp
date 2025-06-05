#include <gtest/gtest.h>
#include "scheduler.h"
//Эти тесты должны падать:

//Проверка простого цикла (A->B->A)
TEST(TaskSchedulerTest, SimpleCycleDetection) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([](int x) { return x; }, 0);
    auto id2 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id1));
    
    EXPECT_THROW({
        scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id2));
    }, TaskScheduler::TaskSchedulerError);
}

//Проверка транзитивного цикла (A->B->C->A)
TEST(TaskSchedulerTest, TransitiveCycleDetection) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([](int x) { return x; }, 0);
    auto id2 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id2));
    
    EXPECT_THROW({
        scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id3));
    }, TaskScheduler::TaskSchedulerError);
}

//Проверка глубоко вложенного цикла
TEST(TaskSchedulerTest, DeepNestedCycle) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([](int x) { return x; }, 0);
    auto id2 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id2));
    auto id4 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id3));
    auto id5 = scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id4));
    
    EXPECT_THROW({
        scheduler.add([](int x) { return x; }, scheduler.getFutureResult<int>(id5));
    }, TaskScheduler::TaskSchedulerError);
}