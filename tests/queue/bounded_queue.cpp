#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "queue/bounded_queue.hpp"

TEST(BasicCheck, SimpleCheck) { EXPECT_EQ(1 + 1, 2); }

TEST(BoundedQueueTest, PushAndPopFifo) {
    dispatcher::queue::BoundedQueue queue(2);
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

TEST(BoundedQueueTest, EmptyQueueReturnsNullopt) {
    dispatcher::queue::BoundedQueue queue(1);
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(BoundedQueueTest, CapacityIsLimited) {
    dispatcher::queue::BoundedQueue queue(2);
    queue.push([]() {});
    queue.push([]() {});

    bool pushed = false;
    std::thread producer([&]() {
        queue.push([]() {});
        pushed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(pushed);

    auto task = queue.try_pop();
    ASSERT_TRUE(task.has_value());
    producer.join();
    EXPECT_TRUE(pushed);

    ASSERT_TRUE(queue.try_pop().has_value());
    ASSERT_TRUE(queue.try_pop().has_value());
}
