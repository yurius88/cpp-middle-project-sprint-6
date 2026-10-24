#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(int capacity) : free_slots_(capacity), used_slots_(0) {}

void BoundedQueue::push(std::function<void()> task) {
    free_slots_.acquire();
    {
        std::lock_guard lock(mutex_);
        queue_.push(std::move(task));
    }
    used_slots_.release();
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    if (!used_slots_.try_acquire()) {
        return std::nullopt;
    }
    std::function<void()> task;
    {
        std::lock_guard lock(mutex_);
        task = std::move(queue_.front());
        queue_.pop();
    }
    free_slots_.release();
    return task;
}

BoundedQueue::~BoundedQueue() = default;

}  // namespace dispatcher::queue
