#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(int capacity) : free_slots_(capacity) {}

void BoundedQueue::push(std::function<void()> task) {
    free_slots_.acquire();
    {
        std::lock_guard lock(mutex_);
        queue_.push(std::move(task));
    }
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    std::unique_lock lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    std::function<void()> task = std::move(queue_.front());
    queue_.pop();
    lock.unlock();
    free_slots_.release();
    return task;
}

BoundedQueue::~BoundedQueue() = default;

}  // namespace dispatcher::queue
