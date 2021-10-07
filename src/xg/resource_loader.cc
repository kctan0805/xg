// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/resource_loader.h"

#include <cassert>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "xg/device.h"
#include "xg/logger.h"
#include "xg/queue.h"
#include "xg/thread_pool.h"
#include "xg/utility.h"

namespace xg {

std::shared_ptr<Device> ResourceLoader::device_;
std::vector<std::shared_ptr<ResourceLoaderContext>> ResourceLoader::contexts_;
std::list<std::shared_ptr<ResourceLoaderContext>> ResourceLoader::contexts_lru_;
std::mutex ResourceLoader::context_mutex_;

xg::ResourceLoaderContext::~ResourceLoaderContext() {
  if (queue) {
    queue->WaitIdle();
    queue.reset();
  }
  cmd_buffer.reset();
}

bool ResourceLoader::Initialize(const ResourceLoaderInfo& info) {
  for (auto i = 0; i < info.queues.size(); ++i) {
    const auto ctxt = std::make_shared<ResourceLoaderContext>();
    if (!ctxt) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return false;
    }

    ctxt->queue = info.queues[i];
    ctxt->cmd_pool = info.cmd_pools[i];
    ctxt->cmd_buffer = info.cmd_buffers[i];
    ctxt->load_complete_fence = info.fences[i];

    SubmitInfo submit_info = {};
    submit_info.cmd_buffers.resize(1);
    ctxt->queue_submit_info.submit_infos.emplace_back(std::move(submit_info));
    ctxt->queue_submit_info.fence = ctxt->load_complete_fence.get();

    contexts_.emplace_back(ctxt);
    contexts_lru_.emplace_back(ctxt);
  }
  device_ = info.device;

  return true;
}

void ResourceLoader::Terminate() {
  contexts_lru_.clear();
  contexts_.clear();
  device_ = nullptr;
}

ResourceLoaderContext* ResourceLoader::AcquireNextContext(
    std::shared_ptr<Task> task) {
  assert(task != nullptr);
  ResourceLoaderContext* context = nullptr;

  auto find_ctxt_func = [](std::shared_ptr<ResourceLoaderContext> ctxt) {
    auto loader = std::static_pointer_cast<ResourceLoader>(ctxt->loader);
    if (loader) {
      loader->UpdateStatus();
      const auto status = loader->GetStatus();
      if (status == ResourceLoaderStatus::kFinished) {
        assert(!ctxt->loader);
        return true;
      } else if (status == ResourceLoaderStatus::kCompleted) {
        loader->Finish();
        return true;
      }
      return false;
    } else {
      return true;
    }
  };

  {
    std::lock_guard<std::mutex> lock(context_mutex_);
    for (;;) {
      auto it = std::find_if(contexts_lru_.begin(), contexts_lru_.end(),
                             find_ctxt_func);
      if (it == contexts_lru_.end()) continue;

      assert((*it)->loader == nullptr);

      contexts_lru_.splice(contexts_lru_.end(), contexts_lru_,
                           it);  // move to the end
      context = it->get();
      break;
    }

    context->loader = task;
  }
  context->load_complete_fence->Reset();
  context->staging_buffer.reset();
  context->cmd_buffer->Reset();

  return context;
}

void ResourceLoader::UpdateStatus() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (status_ == ResourceLoaderStatus::kEnded) {
    if (context_->load_complete_fence->IsSignaled())
      status_ = ResourceLoaderStatus::kCompleted;
  }
}

void ResourceLoader::Finish() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (status_ == ResourceLoaderStatus::kFinished) return;

  auto future = barrier_.get_future();
  future.wait();

  context_->load_complete_fence->Wait();
  context_->loader.reset();
  context_ = nullptr;
  status_ = ResourceLoaderStatus::kFinished;
}

}  // namespace xg
