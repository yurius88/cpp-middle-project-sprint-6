#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(std::size_t thread_count,
                               const std::map<TaskPriority, queue::QueueOptions> &queue_options)
    : queue_(std::make_shared<queue::PriorityQueue>(queue_options)),
      pool_(std::make_unique<thread_pool::ThreadPool>(queue_, thread_count)) {}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) {
    queue_->push(priority, std::move(task));
}

TaskDispatcher::~TaskDispatcher() = default;

}  // namespace dispatcher