#include <gtest/gtest.h>
#include "scheduler.h"
#include <string>
#include <memory>

//Проверка поддержки разных типов возвращаемых значений
TEST(TaskSchedulerTest, MultipleTypes) {
    TaskScheduler scheduler;
    
    auto int_id = scheduler.add([](int x) { return x + 1; }, 5);
    auto str_id = scheduler.add([](std::string s) { return s + "!"; }, "hello");
    
    scheduler.executeAll();
    
    EXPECT_EQ(scheduler.getResult<int>(int_id), 6);
    EXPECT_EQ(scheduler.getResult<std::string>(str_id), "hello!");
}

//Проверка несоответствия типов
TEST(TaskSchedulerTest, TypeMismatch) {
    TaskScheduler scheduler;
    auto id = scheduler.add([] { return 42; });
    EXPECT_THROW(scheduler.getResult<std::string>(id), TaskScheduler::TaskSchedulerError);
}

//Проверка работы с shared_ptr
TEST(TaskSchedulerTest, SharedPtrType) {
    TaskScheduler scheduler;
    
    auto id = scheduler.add([](std::shared_ptr<int> p) { return *p + 1; }, 
        std::make_shared<int>(5));
    
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 6);
}

//Проверка работы с rvalue ссылками
TEST(TaskSchedulerTest, RvalueReference) {
    TaskScheduler scheduler;
    
    std::string base = "hello";
    auto id = scheduler.add([](std::string s) { return s + "!"; }, std::move(base));
    
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<std::string>(id), "hello!");
    EXPECT_TRUE(base.empty());
}

//Проверка работы с различными числовыми типами
TEST(TaskSchedulerTest, DifferentNumericTypes) {
    TaskScheduler scheduler;
    
    auto id1 = scheduler.add([](short a) { return a + 1;}, static_cast<short>(5));
    auto id2 = scheduler.add([](double a) { return a + 0.5;}, 1.0);
    
    scheduler.executeAll();
    
    EXPECT_EQ(scheduler.getResult<int>(id1), 6);
    EXPECT_DOUBLE_EQ(scheduler.getResult<double>(id2), 1.5);
}

//Проверка работы с move-only типами
TEST(TaskSchedulerTest, MoveOnlyTypeWithRef) {
    TaskScheduler scheduler;
    
    auto ptr = std::make_unique<int>(5);
    auto id = scheduler.add(
        [](std::unique_ptr<int>& p) { return *p + 1;},
        std::ref(ptr)
    );
    
    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 6);
}