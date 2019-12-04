// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_THREAD_POOL_H_
#define XG_THREAD_POOL_H_

#pragma warning(push)
#pragma warning(disable : 4146)
#include "thread_pool/thread_pool.hpp"
#pragma warning(pop)

#include <future>
#include <memory>
#include <mutex>
#include <thread>

namespace xg {

class Task {
 public:
  Task() = default;
  virtual ~Task() = default;
  Task(const Task&) = default;
  Task& operator=(const Task&) = default;
  Task(Task&&) = default;
  Task& operator=(Task&&) = default;

  virtual void Run(std::shared_ptr<Task> self) {}

  virtual void* Finish() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& future = barrier_.get_future();
    future.wait();
    auto result = future.get();
    return result;
  }

 protected:
  std::mutex mutex_;
  std::promise<void*> barrier_;
};

class ThreadPool {
 public:
  static ThreadPool& Get() {
    static ThreadPool thread_pool;
    return thread_pool;
  }

  ~ThreadPool();

  class Job {
   public:
    Job(std::shared_ptr<Task> task) : task_(task) {}
    Job(const Job&) = default;
    Job& operator=(const Job&) = default;
    Job(Job&&) = default;
    Job& operator=(Job&&) = default;

    void operator()() { task_->Run(task_); }

   private:
    std::shared_ptr<Task> task_;
  };

  size_t GetCurrentWorkerId() {
    return *tp::detail::thread_id();
  }

  size_t GetWorkerCount() { return worker_count_; }

  template <typename Task>
  void Post(Task&& task) {
    thread_pool_->post(std::forward<Task>(task));
  }

 private:
  ThreadPool();
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  std::unique_ptr<tp::ThreadPool> thread_pool_;
  size_t worker_count_ = 0;
};

}  // namespace xg

#endif  // XG_THREAD_POOL_H_
