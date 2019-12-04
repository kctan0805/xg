// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/thread_pool.h"

#include <cassert>
#include <thread>

#include "thread_pool/thread_pool.hpp"
#include "xg/logger.h"

namespace xg {

ThreadPool::ThreadPool() {
  tp::ThreadPoolOptions options;

  thread_pool_ = std::make_unique<tp::ThreadPool>(options);
  if (!thread_pool_) {
    XG_ERROR("create thread poll fail");
    assert(0);
    return;
  }
  worker_count_ = options.threadCount();
}

ThreadPool::~ThreadPool() { thread_pool_.reset(); }

}  // namespace xg
