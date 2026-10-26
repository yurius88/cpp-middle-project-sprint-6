#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace dispatcher::queue {

class PriorityQueue {
public:
    explicit PriorityQueue(const std::map<TaskPriority, QueueOptions> &options);

    void push(TaskPriority priority, std::function<void()> task);
    std::optional<std::function<void()>> pop();

    void shutdown();

    ~PriorityQueue();

private:
    std::map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

}  // namespace dispatcher::queue
