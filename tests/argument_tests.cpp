#include <gtest/gtest.h>
#include "scheduler.h"

//Проверка максимального количества аргументов(2)
TEST(TaskSchedulerTest, TwoArguments) {
    TaskScheduler scheduler;
    auto id = scheduler.add([](int a, float b) { return a + b; }, 1, 2.5f);
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 3.5f);
}

//Проверка задачи с одним аргументом
TEST(TaskSchedulerTest, OneArgument) {
    TaskScheduler scheduler;
    auto id = scheduler.add([](int a) { return a * 2; }, 5);
    EXPECT_EQ(scheduler.getResult<int>(id), 10);
}

//Проверка задачи без аргументов
TEST(TaskSchedulerTest, ZeroArguments) {
    TaskScheduler scheduler;
    auto id = scheduler.add([] { return 42; });
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 42);
}

//Проверка работы с константными аргументами
TEST(TaskSchedulerTest, ConstArguments) {
    TaskScheduler scheduler;
    
    const int x = 5;
    auto id = scheduler.add([](const int& v) { return v * 2; }, x);
    
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 10);
}

//Проверка передачи аргумента по ссылке
TEST(TaskSchedulerTest, ReferenceArgument) {
    TaskScheduler scheduler;
    int value = 10;
    auto id = scheduler.add([](int& v) { return ++v; }, std::ref(value));
    scheduler.executeAll();
    EXPECT_EQ(value, 11);
}