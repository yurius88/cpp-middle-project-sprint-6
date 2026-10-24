#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <map>
#include <thread>
#include <vector>

#include "task_dispatcher.hpp"
#include "types.hpp"

using namespace dispatcher;

TEST(BasicCheck, SimpleCheck) { EXPECT_EQ(1 + 1, 2); }

TEST(TaskDispatcherTest, AllScheduledTasksAreExecuted) {
    std::atomic<int> executed = 0;
    const int task_count = 100;
    {
        TaskDispatcher dispatcher(4);
        for (int i = 0; i < task_count; ++i) {
            dispatcher.schedule(TaskPriority::Normal, [&executed]() { executed.fetch_add(1); });
            dispatcher.schedule(TaskPriority::High, [&executed]() { executed.fetch_add(1); });
        }
    }
    // деструктор дожидается завершения всех задач в потоках пула
    EXPECT_EQ(executed.load(), 2 * task_count);
}

TEST(TaskDispatcherTest, CustomQueueConfiguration) {
    std::map<TaskPriority, queue::QueueOptions> options = {
        {TaskPriority::High, {.bounded = true, .capacity = 10}},
        {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
    };
    std::atomic<int> executed = 0;
    {
        TaskDispatcher dispatcher(2, options);
        for (int i = 0; i < 50; ++i) {
            dispatcher.schedule(TaskPriority::High, [&executed]() { executed.fetch_add(1); });
            dispatcher.schedule(TaskPriority::Normal, [&executed]() { executed.fetch_add(1); });
        }
    }
    EXPECT_EQ(executed.load(), 100);
}

TEST(TaskDispatcherTest, TaskExceptionDoesNotBreakPool) {
    std::atomic<int> executed = 0;
    {
        TaskDispatcher dispatcher(2);
        dispatcher.schedule(TaskPriority::High, []() { throw std::runtime_error("failure"); });
        dispatcher.schedule(TaskPriority::Normal, [&executed]() { executed.fetch_add(1); });
    }
    EXPECT_EQ(executed.load(), 1);
}

TEST(TaskDispatcherTest, TasksFromMultipleThreads) {
    std::atomic<int> executed = 0;
    {
        TaskDispatcher dispatcher(4);
        std::vector<std::thread> producers;
        for (int i = 0; i < 5; ++i) {
            producers.emplace_back([&dispatcher, &executed]() {
                for (int j = 0; j < 20; ++j) {
                    dispatcher.schedule(TaskPriority::Normal, [&executed]() { executed.fetch_add(1); });
                    dispatcher.schedule(TaskPriority::High, [&executed]() { executed.fetch_add(1); });
                }
            });
        }
        for (auto &producer : producers) {
            producer.join();
        }
    }
    EXPECT_EQ(executed.load(), 200);
}
