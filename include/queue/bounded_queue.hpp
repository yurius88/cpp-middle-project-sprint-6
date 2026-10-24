#pragma once
#include "queue/queue.hpp"

#include <mutex>
#include <queue>
#include <semaphore>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
public:
    explicit BoundedQueue(int capacity);

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~BoundedQueue() override;

private:
    std::mutex mutex_;
    std::queue<std::function<void()>> queue_;
    std::counting_semaphore<> free_slots_;
};

}  // namespace dispatcher::queue
