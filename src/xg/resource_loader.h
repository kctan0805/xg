// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_RESOURCE_LOADER_H_
#define XG_RESOURCE_LOADER_H_

#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "xg/command_buffer.h"
#include "xg/command_pool.h"
#include "xg/device.h"
#include "xg/fence.h"
#include "xg/queue.h"
#include "xg/thread_pool.h"

namespace xg {

class ResourceLoader;

struct ResourceLoaderContext {
  std::shared_ptr<Queue> queue;
  std::shared_ptr<CommandPool> cmd_pool;
  std::shared_ptr<CommandBuffer> cmd_buffer;
  std::shared_ptr<Fence> load_complete_fence;
  std::shared_ptr<Buffer> staging_buffer;
  std::shared_ptr<Task> loader;
  QueueSubmitInfo queue_submit_info;

  ~ResourceLoaderContext();
};

enum class ResourceLoaderStatus {
  kUndefined,
  kRunning,
  kEnded,
  kCompleted,
  kFinished
};

struct ResourceLoaderInfo {
  std::shared_ptr<Device> device;
  std::vector<std::shared_ptr<Queue>> queues;
  std::vector<std::shared_ptr<CommandPool>> cmd_pools;
  std::vector<std::shared_ptr<CommandBuffer>> cmd_buffers;
  std::vector<std::shared_ptr<Fence>> fences;
};

class ResourceLoader : public Task {
 public:
  static bool Initialize(const ResourceLoaderInfo& info);
  static void Terminate();
  static const std::shared_ptr<Device> GetDevice() { return device_; }
  static ResourceLoaderContext* AcquireNextContext(std::shared_ptr<Task> task);

  virtual ResourceLoaderStatus GetStatus();
  void Finish() override;
  int GetResult() const { return result_; }

 protected:
  ResourceLoaderContext* context_ = nullptr;
  ResourceLoaderStatus status_ = ResourceLoaderStatus::kUndefined;
  int result_ = -1;
  std::mutex mutex_;

private:
  static std::shared_ptr<Device> device_;
  static std::vector<std::shared_ptr<ResourceLoaderContext>> contexts_;
  static std::list <std::shared_ptr<ResourceLoaderContext>> contexts_lru_;
  static std::mutex context_mutex_;
};

}  // namespace xg

#endif  // XG_RESOURCE_LOADER_H_
