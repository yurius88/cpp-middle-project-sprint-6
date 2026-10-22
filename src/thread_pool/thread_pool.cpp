#include "thread_pool/thread_pool.hpp"

#include "logger.hpp"

#include <exception>

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, std::size_t thread_count)
    : queue_(std::move(queue)) {
    workers_.reserve(thread_count);
    for (std::size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this]() { worker_loop(); });
    }
}

void ThreadPool::worker_loop() {
    while (true) {
        auto task = queue_->pop();
        if (!task.has_value()) {
            return;
        }
        try {
            (*task)();
        } catch (const std::exception &e) {
            Logger::Get().Log(std::string("Task failed with exception: ") + e.what());
        } catch (...) {
            Logger::Get().Log("Task failed with unknown exception");
        }
    }
}

ThreadPool::~ThreadPool() {
    // потоки дорабатывают оставшиеся задачи и завершаются, когда pop вернёт std::nullopt
    queue_->shutdown();
    for (auto &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

}  // namespace dispatcher::thread_pool