#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <map>
#include <optional>
#include <thread>
#include <vector>

#include "queue/priority_queue.hpp"
#include "types.hpp"

using namespace dispatcher;
using namespace dispatcher::queue;

TEST(PriorityQueueTest, HighPriorityExtractedFirst) {
    std::map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {.bounded = false, .capacity = std::nullopt}},
        {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
    };
    PriorityQueue queue(options);

    std::vector<int> order;
    queue.push(TaskPriority::Normal, [&order]() { order.push_back(0); });
    queue.push(TaskPriority::High, [&order]() { order.push_back(1); });

    auto first = queue.pop();
    ASSERT_TRUE(first.has_value());
    (*first)();
    auto second = queue.pop();
    ASSERT_TRUE(second.has_value());
    (*second)();
    EXPECT_EQ(order, (std::vector<int>{1, 0}));
}

TEST(PriorityQueueTest, PopBlocksUntilTaskArrives) {
    std::map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {.bounded = false, .capacity = std::nullopt}},
        {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
    };
    PriorityQueue queue(options);

    bool popped = false;
    std::jthread consumer([&]() {
        auto task = queue.pop();
        if (task.has_value()) {
            (*task)();
            popped = true;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(popped);

    queue.push(TaskPriority::Normal, []() {});
    consumer.join();
    EXPECT_TRUE(popped);
}

TEST(PriorityQueueTest, ShutdownUnblocksPop) {
    std::map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {.bounded = false, .capacity = std::nullopt}},
        {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
    };
    PriorityQueue queue(options);

    std::optional<std::function<void()>> result;
    std::thread consumer([&]() { result = queue.pop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.shutdown();
    consumer.join();
    EXPECT_FALSE(result.has_value());
}

TEST(PriorityQueueTest, PushAfterShutdownThrows) {
    std::map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {.bounded = false, .capacity = std::nullopt}},
        {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
    };
    PriorityQueue queue(options);
    queue.shutdown();
    EXPECT_THROW(queue.push(TaskPriority::Normal, []() {}), std::runtime_error);
}
