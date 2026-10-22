#pragma once
#include "queue/priority_queue.hpp"

#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

namespace dispatcher::thread_pool {

class ThreadPool {
public:
    explicit ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, std::size_t thread_count);
    ~ThreadPool();

private:
    void worker_loop();

    std::shared_ptr<queue::PriorityQueue> queue_;
    std::vector<std::thread> workers_;
};

}  // namespace dispatcher::thread_pool