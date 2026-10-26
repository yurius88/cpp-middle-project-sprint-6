#include <gtest/gtest.h>

#include "queue/unbounded_queue.hpp"

TEST(UnboundedQueueTest, PushAndPopFifo) {
    dispatcher::queue::UnboundedQueue queue;
    int counter = 0;
    queue.push([&counter]() { counter += 1; });
    queue.push([&counter]() { counter += 10; });

    auto first = queue.try_pop();
    ASSERT_TRUE(first.has_value());
    (*first)();
    EXPECT_EQ(counter, 1);

    auto second = queue.try_pop();
    ASSERT_TRUE(second.has_value());
    (*second)();
    EXPECT_EQ(counter, 11);
}

TEST(UnboundedQueueTest, EmptyQueueReturnsNullopt) {
    dispatcher::queue::UnboundedQueue queue;
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(UnboundedQueueTest, NoCapacityLimit) {
    dispatcher::queue::UnboundedQueue queue;
    const int task_count = 1000;
    for (int i = 0; i < task_count; ++i) {
        queue.push([]() {});
    }
    for (int i = 0; i < task_count; ++i) {
        ASSERT_TRUE(queue.try_pop().has_value());
    }
    EXPECT_FALSE(queue.try_pop().has_value());
}
