#pragma once

#include <cstddef>
#include <map>
#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
public:
    explicit TaskDispatcher(std::size_t thread_count,
                            const std::map<TaskPriority, queue::QueueOptions> &queue_options = {
                                {TaskPriority::High, {.bounded = true, .capacity = 1000}},
                                {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
                            });

    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher();

private:
    std::shared_ptr<queue::PriorityQueue> queue_;
    std::unique_ptr<thread_pool::ThreadPool> pool_;
};

}  // namespace dispatcher