#include <gtest/gtest.h>
#include "scheduler.h"
#include <string>

//Проверка вычисления квадратного уравнения
TEST(TaskSchedulerTest, QuadraticEquation) {
    TaskScheduler scheduler;
    float a = 1, b = -2, c = 0;
    
    auto id1 = scheduler.add([](float a, float c) { return -4 * a * c; }, a, c);
    auto id2 = scheduler.add([](float b, float v) { return b * b + v; }, b, scheduler.getFutureResult<float>(id1));
    auto id3 = scheduler.add([](float b, float d) { return -b + std::sqrt(d); }, b, scheduler.getFutureResult<float>(id2));
    auto id4 = scheduler.add([](float b, float d) { return -b - std::sqrt(d); }, b, scheduler.getFutureResult<float>(id2));
    auto id5 = scheduler.add([](float a, float v) { return v / (2 * a); }, a, scheduler.getFutureResult<float>(id3));
    auto id6 = scheduler.add([](float a, float v) { return v / (2 * a); }, a, scheduler.getFutureResult<float>(id4));
    
    scheduler.executeAll();
    
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id5), 2.0f);
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id6), 0.0f);
}

//Проверка работы с методами класса
TEST(TaskSchedulerTest, ClassMethod) {
    struct AddNumber {
        float add(float a) const { return a + number; }
        float number;
    };

    TaskScheduler scheduler;
    AddNumber adder{.number = 3};
    
    auto id = scheduler.add(&AddNumber::add, adder, 5.0f);
    scheduler.executeAll();
    
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 8.0f);
}

//Проверка работы с неконстантным методом класса
TEST(TaskSchedulerTest, NonConstClassMethod) {
    struct Counter {
        int increment() { return ++count; }
        int count = 0;
    };

    TaskScheduler scheduler;
    Counter counter;
    
    auto id = scheduler.add(&Counter::increment, std::ref(counter));
    scheduler.executeAll();
    
    EXPECT_EQ(scheduler.getResult<int>(id), 1);
    EXPECT_EQ(counter.count, 1);
}

//Проверка ленивого выполнения задач
TEST(TaskSchedulerTest, LazyExecution) {
    TaskScheduler scheduler;
    bool executed = false;
    
    auto id = scheduler.add([&executed](int x) { 
        executed = true; 
        return x * 2; 
    }, 5);
    
    EXPECT_FALSE(executed);
    EXPECT_EQ(scheduler.getResult<int>(id), 10);
    EXPECT_TRUE(executed);
}

//Проверка порядка выполнения зависимых задач
TEST(TaskSchedulerTest, ExecutionOrder) {
    TaskScheduler scheduler;
    std::vector<int> execution_order;
    
    auto id1 = scheduler.add([&] { 
        execution_order.push_back(1);
        return 0; 
    });
    auto id2 = scheduler.add([&](int) { 
        execution_order.push_back(2);
        return 0;
    }, scheduler.getFutureResult<int>(id1));
    
    scheduler.executeAll();
    
    EXPECT_EQ(execution_order, (std::vector<int>{1, 2}));
}

//Проверка повторного использования результатов
TEST(TaskSchedulerTest, ResultReuse) {
    TaskScheduler scheduler;
    int executionCount = 0;
    
    auto id = scheduler.add([&executionCount](int x) { 
        executionCount++; 
        return x * 2; 
    }, 5);
    
    scheduler.getResult<int>(id);
    scheduler.getResult<int>(id);
    scheduler.executeAll();
    scheduler.getResult<int>(id);
    
    EXPECT_EQ(executionCount, 1);
}

//Проверка работы с функтором(объектом с оператором вызова)
TEST(TaskSchedulerTest, FunctorSupport) {
    struct Multiplier {
        int operator()(int x, int y) const { return x * y; }
    };

    TaskScheduler scheduler;
    Multiplier mult;
    
    auto id = scheduler.add(mult, 5, 3);
    scheduler.executeAll();
    
    EXPECT_EQ(scheduler.getResult<int>(id), 15);
}

//Проверка задачи с несколькими зависимостями
TEST(TaskSchedulerTest, MultipleDependencies) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([] { return 2; });
    auto id2 = scheduler.add([] { return 3; });
    auto id3 = scheduler.add([](int a, int b) { return a + b;}, 
                           scheduler.getFutureResult<int>(id1),
                           scheduler.getFutureResult<int>(id2));
    
    scheduler.executeAll();
    
    EXPECT_EQ(scheduler.getResult<int>(id3), 5);
}