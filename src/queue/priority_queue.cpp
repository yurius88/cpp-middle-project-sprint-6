#include "queue/priority_queue.hpp"

#include <stdexcept>

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const std::map<TaskPriority, QueueOptions> &options) {
    for (const auto &[priority, queue_options] : options) {
        if (queue_options.bounded) {
            if (!queue_options.capacity.has_value() || *queue_options.capacity <= 0) {
                throw std::invalid_argument("bounded queue requires positive capacity");
            }
            queues_.emplace(priority, std::make_unique<BoundedQueue>(*queue_options.capacity));
        } else {
            queues_.emplace(priority, std::make_unique<UnboundedQueue>(0));
        }
    }
}

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    IQueue *queue = nullptr;
    {
        std::lock_guard lock(mutex_);
        auto it = queues_.find(priority);
        if (it == queues_.end()) {
            throw std::invalid_argument("unknown task priority");
        }
        queue = it->second.get();
    }
    // потенциально блокирующий push вне mutex_: избегаем инверсии порядка блокировок
    queue->push(std::move(task));
    cv_.notify_all();
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock lock(mutex_);
    while (true) {
        for (auto &[priority, queue] : queues_) {
            if (auto task = queue->try_pop()) {
                return task;
            }
        }
        if (stopped_) {
            return std::nullopt;
        }
        cv_.wait(lock);
    }
}

void PriorityQueue::shutdown() {
    {
        std::lock_guard lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();
}

PriorityQueue::~PriorityQueue() { shutdown(); }

}  // namespace dispatcher::queue
